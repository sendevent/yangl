/*
   Copyright (C) 2025-2026 Denis Gofman - <sendevent@gmail.com>

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

ServerLocationResolver::ServerLocationResolver(ActionStorage * /*actionStorage*/, QObject *parent)
    : QObject(parent)
    , m_listManager(new ServersListManager(this))
    , m_geoResolver(new CoordinatesResolver(this))
{
    connect(m_listManager, &ServersListManager::citiesAdded, this, &ServerLocationResolver::resolveServers);
    connect(m_listManager, &ServersListManager::discoveryProgress, this, &ServerLocationResolver::progressChanged);
    connect(m_listManager, &ServersListManager::ready, this, &ServerLocationResolver::onDiscoveryComplete);
    connect(m_geoResolver, &CoordinatesResolver::coordinatesResolved, this, &ServerLocationResolver::onPlaceResolved);
}

void ServerLocationResolver::resolveServers(const Places &places)
{
    m_serversFound += places.size();
    for (const auto &place : places) {
        if (!place.isGroup()) {
            m_refreshedCountries.insert(utils::geoToNvpn(place.country).toLower());
        }
        resolveServerLocation(place);
    }
}

void ServerLocationResolver::resolveServerLocation(const PlaceInfo &place)
{
    LOG << place.country << place.town << place.ok << place.location << place.location.isValid();
    ensureCacheLoaded();

    const auto &countryName = place.country.toLower();
    const auto &cityName = place.town.toLower();

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
    if (place.ok) {
        m_placesChecked[place.country.toLower()].insert(place.town.toLower(), place);
    } else {
        WRN << place.message;
    }

    notifyPlace(place);
}

static QString geoCacheFilePath()
{
    static QString path = QString("%1/servers.json").arg(SettingsManager::dirPath());
    return path;
}

bool ServerLocationResolver::ensureCacheLoaded()
{
    if (!m_cacheLoaded) {
        m_serversFound = 0;
        m_serversResolved = 0;

        m_cacheLoaded = true;
        loadCache();
    }

    if (!m_serversFound || m_serversFound != m_serversResolved) {
        return true;
    }

    QFileInfo fi(geoCacheFilePath());
    if (fi.lastModified().daysTo(QDateTime::currentDateTime()) < 1) {
        m_placesChecked = m_placesLoaded;
        return false;
    }

    return true;
}

static constexpr qint64 CacheTTLSecs = 7 * 24 * 3600; // 7 days

namespace JsonConsts {
static const QLatin1String Country { "country" };
static const QLatin1String City { "city" };
static const QLatin1String Lat { "lat" };
static const QLatin1String Lon { "lon" };
static const QLatin1String Capital { "capital" };
static const QLatin1String Places { "places" };
static const QLatin1String Timestamps { "timestamps" };
static const QLatin1String NordVpnVersion { "nordvpnVersion" };
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

    QJsonArray jArr;
    if (jDoc.isArray()) {
        jArr = jDoc.array();
    } else if (jDoc.isObject()) {
        const auto &root = jDoc.object();
        jArr = root[JsonConsts::Places].toArray();
        m_cachedNordVpnVersion = root[JsonConsts::NordVpnVersion].toString();
        const auto &timestamps = root[JsonConsts::Timestamps].toObject();
        for (auto it = timestamps.begin(); it != timestamps.end(); ++it) {
            m_countryTimestamps[it.key()] = it.value().toInteger();
        }
    }

    m_serversFound = jArr.size();

    for (const auto &jVal : jArr) {
        const auto &jObj = jVal.toObject();
        const PlaceInfo place {
            jObj[JsonConsts::Country].toString(),
            jObj[JsonConsts::City].toString(),
            QGeoCoordinate { jObj[JsonConsts::Lat].toDouble(), jObj[JsonConsts::Lon].toDouble() },
            jObj[JsonConsts::Capital].toString().toLower() == "true",
            true, // ok
        };
        m_placesLoaded[place.country.toLower()].insert(place.town.toLower(), place);

        notifyPlace(place);
    }
}

void ServerLocationResolver::saveCache()
{
    if (m_placesLoaded == m_placesChecked && m_refreshedCountries.isEmpty()) {
        return;
    }

    const qint64 now = QDateTime::currentSecsSinceEpoch();
    for (const auto &country : m_refreshedCountries) {
        m_countryTimestamps[country] = now;
    }
    m_refreshedCountries.clear();

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

    QJsonObject timestamps;
    for (auto it = m_countryTimestamps.cbegin(); it != m_countryTimestamps.cend(); ++it) {
        timestamps[it.key()] = it.value();
    }

    const QJsonObject root {
        { JsonConsts::Places, jArr },
        { JsonConsts::Timestamps, timestamps },
        { JsonConsts::NordVpnVersion, m_cachedNordVpnVersion },
    };

    const QJsonDocument jDoc(root);
    const QByteArray &data = jDoc.toJson();
    if (-1 == out.write(std::move(data))) {
        WRN << "error during file write:" << out.errorString();
    }
}

void ServerLocationResolver::refresh()
{
    const bool needActualization = ensureCacheLoaded();

    if (needActualization) {
        const QString currentVersion = m_listManager->queryVersion();
        const bool versionChanged = !currentVersion.isEmpty() && !m_cachedNordVpnVersion.isEmpty()
                && currentVersion != m_cachedNordVpnVersion;
        if (versionChanged) {
            LOG << "NordVPN version changed:" << m_cachedNordVpnVersion << "->" << currentVersion;
            m_countryTimestamps.clear();
        }
        m_cachedNordVpnVersion = currentVersion;

        const auto fresh = freshCountries();

        // Carry over cached data for countries that are still fresh
        for (const auto &nvpnKey : fresh) {
            const QString geoKey = utils::nvpnToGeo(nvpnKey).toLower();
            if (m_placesLoaded.contains(geoKey)) {
                m_placesChecked[geoKey] = m_placesLoaded[geoKey];
            }
        }

        m_serversFound = 0;
        m_serversResolved = 0;
        m_discoveryComplete = false;
        m_refreshedCountries.clear();

        m_listManager->reload(fresh);
    } else if (m_serversFound > 0) {
        emit allResolved();
    }
}

void ServerLocationResolver::notifyPlace(const PlaceInfo &place)
{
    ++m_serversResolved;

    LOG << m_serversResolved << m_serversFound;

    emit serverLocationResolved(place);
    checkCompletion();
}

void ServerLocationResolver::onDiscoveryComplete()
{
    m_discoveryComplete = true;
    checkCompletion();
}

void ServerLocationResolver::checkCompletion()
{
    if (m_discoveryComplete && m_serversFound > 0 && m_serversResolved >= m_serversFound) {
        emit allResolved();
    }
}

QSet<QString> ServerLocationResolver::freshCountries() const
{
    QSet<QString> fresh;
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    for (auto it = m_countryTimestamps.cbegin(); it != m_countryTimestamps.cend(); ++it) {
        if (now - it.value() < CacheTTLSecs) {
            fresh.insert(it.key());
        }
    }
    return fresh;
}
