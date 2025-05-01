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

ServerLocationResolver::ServerLocationResolver(NordVpnWraper *nordVpn, QObject *parent)
    : QObject(parent)
    , m_listManager(new ServersListManager(nordVpn, this))
    , m_geoResolver(new CoordinatesResolver(this))
{
    connect(m_listManager, &ServersListManager::citiesAdded, this, &ServerLocationResolver::resolveServers);
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
    LOG << place.country << place.town;
    ensureCacheLoaded();

    const auto &countryName = place.country.toLower();
    const auto &cityName = place.town.toLower();

    if (m_placesLoaded.contains(countryName)) {
        const auto &cities = m_placesLoaded[countryName];
        if (cities.contains(cityName)) {
            onPlaceResolved(-1, cities.value(cityName));
            return;
        }
    }

    if (place.group) {
        onPlaceResolved(-1, place);
        return;
    }

    m_geoResolver->requestCoordinates(place);
}

void ServerLocationResolver::ensureCacheLoaded()
{
    static bool loadedBuiltin(false);
    if (!loadedBuiltin) {
        loadCache();
        loadedBuiltin = true;
    }
}

void ServerLocationResolver::loadCache() { }

void ServerLocationResolver::saveCache() { }

bool ServerLocationResolver::refresh()
{
    return m_listManager->reload();
}

void ServerLocationResolver::onPlaceResolved(RequestId /*id*/, const PlaceInfo &place)
{
    if (!place.ok) {
        WRN << place.country << place.town << place.message;
        return;
    }

    m_placesChecked[place.country.toLower()].insert(place.town.toLower(), place);
    emit serverLocationResolved(place);
}
