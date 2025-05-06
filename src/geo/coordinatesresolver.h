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

#pragma once

#include "geo/placeinfo.h"

#include <QGeoServiceProvider>
#include <QHash>
#include <QObject>
#include <atomic>

class QGeoCodingManager;

inline bool operator==(const PlaceInfo &lhs, const PlaceInfo &rhs)
{
    return lhs.country == rhs.country && lhs.town == rhs.town && lhs.location == rhs.location;
}

inline uint qHash(const PlaceInfo &key, uint seed = 0)
{
    return qHashMulti(seed, key.country, key.town, key.location.latitude(), key.location.longitude()/*,
                      key.message*/);
}

class CoordinatesResolver : public QObject
{
    Q_OBJECT
public:
    explicit CoordinatesResolver(QObject *parent = nullptr);

    RequestId requestCoordinates(const PlaceInfo &town);
    RequestId requestCoordinates(const QString &country, const QString &city);

signals:
    void coordinatesResolved(RequestId id, const PlaceInfo &town);

private:
    std::atomic<RequestId> m_requestCounter { 0 };

    CitiesByCountry m_data;

    std::unique_ptr<QGeoServiceProvider> m_geoSrvProv;
    QGeoCodingManager *m_geoCoder { nullptr };

    void ensureDataLoaded();

    void lookupForPlaceAsync(const PlaceInfo &request, RequestId id);

    PlaceInfo lookupForPlace(const PlaceInfo &request) const;

    void requestGeoAsync(const PlaceInfo &place, RequestId id);

    static CitiesByCountry loadData(const QString &path);

    friend class TestCoordinatesResolver;
};
