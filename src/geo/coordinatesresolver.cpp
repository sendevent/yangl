/*
   Copyright (C) 2025 Denis Gofman - <sendevent@gmail.com>

This application is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This application is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "coordinatesresolver.h"

#include "app/common.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QFutureSynchronizer>
#include <QFutureWatcher>
#include <QGeoAddress>
#include <QGeoCodeReply>
#include <QGeoCodingManager>
#include <QGeoCoordinate>
#include <QGeoLocation>
#include <QRegularExpression>
#include <QtConcurrentRun>
#include <memory>
#include <qnamespace.h>

static constexpr QChar CSVSeparator(',');
static constexpr size_t CSVColumnCount(5);

CoordinatesResolver::CoordinatesResolver(QObject *parent)
    : QObject { parent }
    , m_geoSrvProv(std::make_unique<QGeoServiceProvider>("osm"))
    , m_geoCoder(m_geoSrvProv->geocodingManager())
{
    if (m_geoCoder) {
        QLocale qLocaleC(QLocale::C, QLocale::AnyCountry);
        m_geoCoder->setLocale(qLocaleC);
    } else {
        WRN << "Can't aquire geocoder" << m_geoSrvProv->geocodingManager();
    }
}

quint32 CoordinatesResolver::requestCoordinates(const PlaceInfo &town)
{
    ensureDataLoaded();

    ++m_requestCounter; // overflow on around 4 billion requests, then goes back to the zero

    lookupForPlaceAsync(town, m_requestCounter);
    return m_requestCounter;
}

quint32 CoordinatesResolver::requestCoordinates(const QString &country, const QString &city)
{
    return requestCoordinates({ country, city });
}

void CoordinatesResolver::ensureDataLoaded()
{
    QFutureSynchronizer<void> synchronizer;

    static bool loadedBuiltin(false);
    if (!loadedBuiltin) {
        synchronizer.addFuture(QtConcurrent::run([this]() {
            const auto &loaded = loadData(":/geo/resources/map/cities.csv");

            if (!loaded.isEmpty()) {
                m_data.insert(loaded);
            }
        }));
        loadedBuiltin = true;
    }

    synchronizer.waitForFinished();
}

CitiesByCountry CoordinatesResolver::loadData(const QString &path)
{
    auto parseLine = [](const QString &line, QChar separator = ',') {
        QStringList result;
        QString field;
        bool insideQuotes = false;

        for (int i = 0; i < line.length(); ++i) {
            const auto &c = line[i];

            if (c == '"') {
                insideQuotes = !insideQuotes;
            } else if (c == separator && !insideQuotes) {
                result.append(field.trimmed());
                field.clear();
            } else {
                field += c;
            }
        }
        result.append(field.trimmed());
        return result;
    };

    CitiesByCountry loaded;

    QFile csv(path);
    if (csv.open(QFile::ReadOnly | QFile::Text)) {
        while (!csv.atEnd()) {
            const auto &line = QString::fromUtf8(csv.readLine());
            const auto &parts = parseLine(line, CSVSeparator);
            if (parts.size() != CSVColumnCount) {
                WRN << QString("Invalid geo line '%1', expected %2 separators (%3) but got %4, ignored")
                                .arg(line, QString::number(CSVColumnCount), CSVSeparator,
                                     QString::number(parts.size()));
                continue;
            }

            const auto [coord, parsed] = utils::parseCoordinates(parts[3], parts[4]);
            if (!parsed) {
                WRN << "Failed parsing lat/lon value:" << parts[3] << parts[4];
                continue;
            }

            const PlaceInfo place {
                parts[0], parts[1], coord, parts[2] == "True", true, QString(),
            };

            auto &country = loaded[place.country.toLower()];
            country.insert(place.town.toLower(), place);
        }

    } else {
        WRN << QString("Places database file '%1' not found: %2").arg(path, csv.errorString());
    }

    return loaded;
}

void CoordinatesResolver::lookupForPlaceAsync(const PlaceInfo &request, RequestId id)
{
    auto future = QtConcurrent::run([this, request]() -> PlaceInfo { return lookupForPlace(request); });

    auto *watcher = new QFutureWatcher<PlaceInfo>(this);
    connect(watcher, &QFutureWatcher<PlaceInfo>::finished, this, [this, id, watcher]() {
        QScopedPointer<QFutureWatcher<PlaceInfo>> cleanup(watcher); // Ensure deletion even if an exception occurs

        const PlaceInfo &placeInfo = watcher->future().result();
        LOG << "Async task finished, place found:" << placeInfo.country << placeInfo.town << placeInfo.ok;
        if (placeInfo.ok) {
            emit coordinatesResolved(id, placeInfo);
        } else {
            requestGeoAsync(placeInfo, id);
        }
    });

    watcher->setFuture(future);
}

PlaceInfo CoordinatesResolver::lookupForPlace(const PlaceInfo &request) const
{
    PlaceInfo town(request);
    town.ok = false;
    town.message = "Not found";

    auto searchForTheCapital = [&town](const auto &place) {
        return place.capital && place.country.toLower() == town.country.toLower();
    };

    const auto &countryName = town.country.toLower();
    const auto &cityName = town.town.toLower();

    if (m_data.contains(countryName)) {
        const auto &country = m_data[countryName];
        if (town.town.isEmpty()) {
            auto it = std::find_if(country.cbegin(), country.cend(), searchForTheCapital);
            if (it != country.end()) {
                town = *it;
                town.ok = true;
            }
        } else if (country.contains(cityName)) {
            town = country.value(cityName);
            town.ok = true;
        }
    }

    return town;
}

void CoordinatesResolver::requestGeoAsync(const PlaceInfo &place, RequestId id)
{
    PlaceInfo result(place);
    result.ok = false;
    result.message = "Not found";

    if (!m_geoCoder) {
        WRN << "GeoCoder is unavailable" << place.country << place.town;
        emit coordinatesResolved(id, result);
        return;
    }

    QGeoAddress addr;
    addr.setCountry(place.country);
    if (place.town != "default") {
        addr.setCity(place.town);
    }
    LOG << addr.country() << addr.city();

    QGeoCodeReply *reply = m_geoCoder->geocode(addr);
    if (!reply) {
        result.message = "Failed to create geocode request!";
        WRN << result.message;
        emit coordinatesResolved(id, result);
        return;
    }

    connect(reply, &QGeoCodeReply::finished, this, [this, reply, id, result]() mutable {
        QScopedPointer<QGeoCodeReply, QScopedPointerDeleteLater> cleanup(reply); // auto deletes reply safely

        if (reply->error() != QGeoCodeReply::NoError) {
            result.message = QString("Geo reply error: %1").arg(reply->errorString());
            emit coordinatesResolved(id, result);
            return;
        }

        const auto &locations = reply->locations();
        if (!locations.isEmpty()) {
            const auto &l = locations.first();
            LOG << result.country << result.town << l.coordinate();
            result.location = l.coordinate();
            result.ok = true;
            result.message.clear();
        } else {
            result.message = QString("No locations found for: `%1` `%2`").arg(result.country, result.town);
            WRN << result.message;
        }

        emit coordinatesResolved(id, result);
    });

    // Optionally handle network errors immediately
    // connect(reply, &QGeoCodeReply::errorOccurred, this, [this, reply, id, result](QGeoCodeReply::Error error) mutable
    // {
    //     WRN << "Geo reply error occurred:" << error << reply->errorString();
    //     reply->deleteLater();
    //     emit coordinatesResolved(id, result);
    // });
}
