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

#include "mapserversmodel.h"

MapServersModel::MapServersModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_coordinates()
{
}

void MapServersModel::clear()
{
    beginResetModel();
    m_coordinates.clear();
    endResetModel();
}

void MapServersModel::addMarker(const PlaceInfo &place)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    m_coordinates.append(place);
    endInsertRows();
}

int MapServersModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_coordinates.count();
}

QVariant MapServersModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >= m_coordinates.count())
        return QVariant();

    const PlaceInfo &info = m_coordinates[row];
    switch (role) {
    case MapServersModel::CountryNameRole:
        return QVariant::fromValue(info.country);
    case MapServersModel::CityNameRole:
        return QVariant::fromValue(info.town);
    case MapServersModel::PositionRole:
        return QVariant::fromValue(info.location);
    }

    return QVariant();
}

QHash<int, QByteArray> MapServersModel::roleNames() const
{
    return {
        { PositionRole, "position" },
        { CountryNameRole, "country" },
        { CityNameRole, "city" },
    };
}
