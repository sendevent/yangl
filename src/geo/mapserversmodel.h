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
#include <qproperty.h>

struct TreeItem {
    QString name;
    PlaceInfo data;
    TreeItem *parent = nullptr;
    std::vector<std::unique_ptr<TreeItem>> children;

    TreeItem *child(int row) const
    {
        if (row >= 0 && row < children.size())
            return children[row].get();
        return nullptr;
    }

    int row() const
    {
        if (!parent)
            return 0;
        for (int i = 0; i < parent->children.size(); ++i) {
            if (parent->children[i].get() == this)
                return i;
        }
        return 0;
    }
};

class MapServersModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        PlaceInfoRole = Qt::UserRole + 1,
    };

    MapServersModel(QObject *parent = nullptr);
    ~MapServersModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override { return 1; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addMarker(const PlaceInfo &place);
    void clear();

    TreeItem *rootItem() const;

private:
    TreeItem *m_root;
};
