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

#include "geo/coordinatesresolver.h"

#include <QIdentityProxyModel>

class QTimer;
class FlatPlaceProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    enum Roles
    {
        PositionRole = Qt::UserRole + 1,
        CountryNameRole,
        CityNameRole
    };

    explicit FlatPlaceProxyModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<PlaceInfo> m_places;

private slots:
    void rebuildFlatList();
    void onRowsInserted(const QModelIndex &parent, int first, int last);
};
