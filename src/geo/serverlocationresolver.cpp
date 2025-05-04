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

#include "serverlocationresolver.h"

#include "app/common.h"
#include "geo/serverslistmanager.h"
#include "settings/settingsmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTimer>
#include <qnamespace.h>

ServerLocationResolver::ServerLocationResolver(NordVpnWraper *nordVpn, QObject *parent)
    : QObject(parent)
    , m_listManager(new ServersListManager(nordVpn, this))
    , m_geoResolver(new CoordinatesResolver(this))
    , m_saveTimer(new QTimer(this))
{
    connect(m_listManager, &ServersListManager::citiesAdded, this, &ServerLocationResolver::resolveServers);
    connect(m_geoResolver, &CoordinatesResolver::coordinatesResolved, this, &ServerLocationResolver::onPlaceResolved);

    connect(this, &ServerLocationResolver::serverLocationResolved, this, &ServerLocationResolver::saveCacheDelayed);

    m_saveTimer->setInterval(60 * utils::oneSecondMs());
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, &QTimer::timeout, this, &ServerLocationResolver::saveCache);
}

void ServerLocationResolver::resolveServers(const Places &places)
{
    for (const auto &place : places) {
        resolveServerLocation(place);
    }
}

void ServerLocationResolver::resolveServerLocation(const PlaceInfo &place)
{
    LOG << place.country << place.town << place.ok << place.location << place.location.isValid();
    ensureCacheLoaded();

    const auto &countryName = place.country;
    const auto &cityName = place.town;

    if (m_placesLoaded.contains(countryName)) {
        const auto &cities = m_placesLoaded[countryName];
        if (cities.contains(cityName)) {
            LOG << place.country << place.town << "found in cache";
            onPlaceResolved(-1, cities.value(cityName));
            return;
        }
    }

    if (place.isGroup()) {
        LOG << place.country << place.town << "it is a group";
        onPlaceResolved(-1, place);
        return;
    }

    LOG << place.country << place.town << "checking online";
    m_geoResolver->requestCoordinates(place);
}

void ServerLocationResolver::ensureCacheLoaded()
{
    static bool loadedBuiltin(false);
    if (!loadedBuiltin) {
        loadedBuiltin = true;
        loadCache();
    }
}

static QString geoCacheFilePath()
{
    static QString path = QString("%1/servers.json").arg(SettingsManager::dirPath());
    LOG << path;
    return path;
}

namespace JsonConsts {
static const QLatin1String Country { "country" };
static const QLatin1String City { "city" };
static const QLatin1String Lat { "lat" };
static const QLatin1String Lon { "lon" };
static const QLatin1String Capital { "capital" };
};

void ServerLocationResolver::loadCache()
{
    static const auto &from = geoCacheFilePath();
    QFile in(from);
    if (!in.open(QFile::ReadOnly | QFile::Text)) {
        WRN << "failed opening file" << from << in.errorString();
        return;
    }

    QJsonParseError err;
    const QByteArray &data = in.readAll();
    const QJsonDocument &jDoc = QJsonDocument::fromJson(std::move(data), &err);
    if (err.error != QJsonParseError::NoError) {
        WRN << "error parsing document:" << err.errorString();
        return;
    }

    const auto &jArr = jDoc.array();
    for (const auto &jObj : jArr) {
        const PlaceInfo place {
            jObj[JsonConsts::Country].toString(),
            jObj[JsonConsts::City].toString(),
            QGeoCoordinate { jObj[JsonConsts::Lat].toDouble(), jObj[JsonConsts::Lon].toDouble() },
            jObj[JsonConsts::Capital].toString().toLower() == "true",
            true, // ok
        };
        m_placesLoaded[place.country].insert(place.town, place);
        emit serverLocationResolved(place);
    }
}

void ServerLocationResolver::saveCache() const
{
    static const auto &to = geoCacheFilePath();
    QFile out(to);
    if (!out.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) || !out.isWritable()) {
        WRN << "failed opening file" << to << out.errorString();
        return;
    }

    QJsonArray jArr;
    for (const auto &country : m_placesChecked) {
        for (const auto &place : country) {
            const QJsonObject jObj {
                { JsonConsts::Country, place.country },
                { JsonConsts::City, place.town },
                { JsonConsts::Lat, place.location.latitude() },
                { JsonConsts::Lon, place.location.longitude() },
                { JsonConsts::Capital, place.capital ? "true" : "false" },
            };
            jArr.append(jObj);
        }
    }

    const QJsonDocument jDoc(jArr);
    const QByteArray &data = jDoc.toJson();
    if (-1 == out.write(std::move(data))) {
        WRN << "error during file write:" << out.errorString();
    }
}

bool ServerLocationResolver::refresh()
{
    ensureCacheLoaded();

    return m_listManager->reload();
}

void ServerLocationResolver::onPlaceResolved(RequestId /*id*/, const PlaceInfo &place)
{
    if (place.ok) {
        LOG << "added" << place.country << place.town << place.location << place.location.isValid();
        m_placesChecked[place.country.toLower()].insert(place.town.toLower(), place);
    } else {
        WRN << place.country << place.town << place.message;
    }

    emit serverLocationResolved(place);
}

void ServerLocationResolver::saveCacheDelayed()
{
    LOG;
    m_saveTimer->stop();
    m_saveTimer->start();
}
