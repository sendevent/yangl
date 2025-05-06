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
#include "geo/coordinatesresolver.h"
#include "geo/serverslistmanager.h"
#include "settings/settingsmanager.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

ServerLocationResolver::ServerLocationResolver(NordVpnWraper *nordVpn, QObject *parent)
    : QObject(parent)
    , m_listManager(new ServersListManager(nordVpn, this))
    , m_geoResolver(new CoordinatesResolver(this))
{
    connect(m_listManager, &ServersListManager::citiesAdded, this, &ServerLocationResolver::resolveServers);
    connect(m_listManager, &ServersListManager::citiesCount, this, [this](int total) { m_serversFound = total; });
    connect(m_geoResolver, &CoordinatesResolver::coordinatesResolved, this, &ServerLocationResolver::onPlaceResolved);
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

void ServerLocationResolver::onPlaceResolved(RequestId id, const PlaceInfo &place)
{
    LOG << id << place.country << place.town;

    if (place.town == "Saint Louis") {
        int dbg = 0;
    }

    if (place.ok) {
        LOG << "added" << place.location << place.location.isValid();
        m_placesChecked[place.country.toLower()].insert(place.town.toLower(), place);
    } else {
        WRN << place.message;
    }

    notifyPlace(place);
}

static QString geoCacheFilePath()
{
    static QString path = QString("%1/servers.json").arg(SettingsManager::dirPath());
    LOG << path;
    return path;
}

bool ServerLocationResolver::ensureCacheLoaded()
{
    bool needsActualization = false;

    if (!m_cacheLoaded) {
        m_serversFound = 0;
        m_serversResolved = 0;

        m_cacheLoaded = true;
        loadCache();
    }

    if (!m_serversFound || !m_serversResolved || m_serversFound != m_serversResolved) {
        needsActualization = true;
    } else {
        QFileInfo fi(geoCacheFilePath());
        needsActualization = fi.lastModified().daysTo(QDateTime::currentDateTime()) >= 1;
    }

    if (!needsActualization) {
        m_placesChecked = m_placesLoaded;
    }

    return needsActualization;
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
    LOG;
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
    m_serversFound = jArr.size();

    for (const auto &jObj : jArr) {
        const PlaceInfo place {
            jObj[JsonConsts::Country].toString(),
            jObj[JsonConsts::City].toString(),
            QGeoCoordinate { jObj[JsonConsts::Lat].toDouble(), jObj[JsonConsts::Lon].toDouble() },
            jObj[JsonConsts::Capital].toString().toLower() == "true",
            true, // ok
        };
        m_placesLoaded[place.country].insert(place.town, place);

        notifyPlace(place);
    }
}

void ServerLocationResolver::saveCache() const
{
    if (m_placesLoaded == m_placesChecked) {
        return;
    }

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

void ServerLocationResolver::refresh()
{
    const bool needActualization = ensureCacheLoaded();

    if (needActualization) {
        m_serversFound = 0;
        m_serversResolved = 0;

        m_listManager->reload();
    }
}

void ServerLocationResolver::notifyPlace(const PlaceInfo &place)
{
    ++m_serversResolved;

    LOG << m_serversResolved << m_serversFound;

    emit serverLocationResolved(place, m_serversResolved, m_serversFound);
}
