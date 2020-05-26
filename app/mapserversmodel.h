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

#pragma once

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

    MapServersModel(QObject *parent = nullptr);
    void addMarker(const QString &country, const QString &city, const QGeoCoordinate &coordinate);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void clear();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    struct Info {
        QString country;
        QString city;
        QGeoCoordinate coordinates;
    };

    QList<Info> m_coordinates;
};
