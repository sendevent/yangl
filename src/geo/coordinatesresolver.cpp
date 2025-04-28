#include "coordinatesresolver.h"

#include "app/common.h"

#include <QCoreApplication>
#include <QFile>
#include <QFutureSynchronizer>
#include <QGeoAddress>
#include <QGeoCodeReply>
#include <QGeoCodingManager>
#include <QGeoCoordinate>
#include <QGeoLocation>
#include <QtConcurrentRun>

static constexpr QChar CSVSeparator(',');
static constexpr size_t CSVColumnCount(5);

CoordinatesResolver::CoordinatesResolver(QGeoCodingManager *geoCoder, QObject *parent)
    : QObject { parent }
    , m_geoCoder(geoCoder)
{
}

void CoordinatesResolver::requestCoordinates(const PlaceInfo &town,
                                             const std::function<void(const PlaceInfo &)> &callback)
{
    ensureDataLoaded();
    auto result = lookupForPlace(town);

    if (!result.ok) {
        result = requestGeo(town);
    }

    if (callback) {
        callback(result);
    }

    emit coordinatesResoloved(result);
}

void CoordinatesResolver::requestCoordinates(const QString &country, const QString &city,
                                             const std::function<void(const PlaceInfo &)> &callback)
{
    return requestCoordinates({ country, city }, callback);
}

void CoordinatesResolver::ensureDataLoaded()
{
    QFutureSynchronizer<void> synchronizer;

    if (!m_loadedBuiltin) {
        synchronizer.addFuture(QtConcurrent::run([this]() { loadDataBuiltin(); }));
    }

    synchronizer.waitForFinished(); // Ensure all tasks complete
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

    auto parseCoordinates = [](const QString &latStr, const QString &lonStr) -> std::tuple<QGeoCoordinate, bool> {
        bool parsed(false);
        QGeoCoordinate coordinate;

        if (!latStr.isEmpty() && !lonStr.isEmpty()) {

            const auto lat = latStr.toDouble(&parsed);
            if (parsed) {
                const auto lon = lonStr.toDouble(&parsed);
                if (parsed) {
                    coordinate = QGeoCoordinate(lat, lon);
                }
            }
        }

        return { coordinate, parsed };
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

            // Finland,Helsinki,True,60.1708,24.9375
            const auto [coord, parsed] = parseCoordinates(parts[3], parts[4]);
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

bool CoordinatesResolver::loadDataBuiltin()
{
    if (!m_loadedBuiltin) {
        const auto &loaded = loadData(":/geo/resources/map/cities.csv");

        if (!loaded.isEmpty()) {
            m_data.insert(loaded);
            m_loadedBuiltin = true;
        }
    }

    return m_loadedBuiltin;
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
                return *it;
            }

        } else if (country.contains(cityName)) {
            town = country.value(cityName);
            town.ok = true;
            return town;
        }
    }

    return town;
}

PlaceInfo CoordinatesResolver::lookupForPlace(const QString &country, const QString &city) const
{
    return lookupForPlace({
            country,
            city,
    });
}

PlaceInfo CoordinatesResolver::requestGeo(const PlaceInfo &place)
{
    PlaceInfo result(place);
    result.ok = false;
    result.message = "Not found";

    QGeoAddress addr;
    addr.setCountry(place.country);
    if (place.town != "default") {
        addr.setCity(place.town);
    }
    LOG << addr.country() << addr.city();

    if (!m_geoCoder) {
        WRN << "GeoCoder is unavailable";
        return result;
    }

    QGeoCodeReply *reply = m_geoCoder->geocode(addr);
    if (!reply) {
        WRN << "Failed to create geocode request!";
        return result;
    }

    LOG << "Geocode requested:" << reply << reply->error() << reply->errorString();

    // Wait for reply to finish
    while (!reply->isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }

    if (reply->error() != QGeoCodeReply::NoError) {
        WRN << "Geo reply error:" << reply->errorString();
        reply->deleteLater();
        return result;
    }

    const auto &locations = reply->locations();
    if (!locations.isEmpty()) {
        const auto &l = locations.first();
        LOG << result.country << result.town << l.coordinate();
        result.location = l.coordinate();
        result.ok = true;
        result.message.clear();
    } else {
        WRN << "No locations found for:" << result.country << result.town;
        result.message = "Not found";
    }

    reply->deleteLater();
    return result;
}
