/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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

#include "geo/coordinatesresolver.h"

#include <QAbstractListModel>
#include <QGeoCoordinate>

class MapServersModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MarkerRoles
    {
        PositionRole = Qt::UserRole + 1,
        CountryNameRole,
        CityNameRole,
    };

    MapServersModel(QObject *parent = {});
    void addMarker(const PlaceInfo &place);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void clear();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    QList<PlaceInfo> m_coordinates;
};
