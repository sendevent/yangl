/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
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
    beginRemoveRows(QModelIndex(), rowCount(), rowCount());
    m_coordinates.clear();
    endRemoveRows();
}

void MapServersModel::addMarker(const QGeoCoordinate &coordinate)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_coordinates.append(coordinate);
    endInsertRows();
}

int MapServersModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_coordinates.count();
}

QVariant MapServersModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_coordinates.count())
        return QVariant();
    if (role == MapServersModel::PositionRole)
        return QVariant::fromValue(m_coordinates[index.row()]);
    return QVariant();
}

QHash<int, QByteArray> MapServersModel::roleNames() const
{
    return { { PositionRole, "position" } };
}
