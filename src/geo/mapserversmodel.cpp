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

#include "app/common.h"
#include "geo/coordinatesresolver.h"

#include <qnamespace.h>

MapServersModel::MapServersModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_root(new TreeItem { "Root" })
{
}

MapServersModel::~MapServersModel()
{
    delete m_root;
}

TreeItem *MapServersModel::rootItem() const
{
    return m_root;
}

QModelIndex MapServersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : m_root;
    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex MapServersModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem *>(index.internalPointer());
    TreeItem *parentItem = childItem->parent;

    if (parentItem == m_root || !parentItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int MapServersModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : m_root;
    return parentItem->children.size();
}

QVariant MapServersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (auto item = static_cast<TreeItem *>(index.internalPointer())) {

        switch (role) {
        case Qt::DisplayRole: {
            return item->name;
        }
        case PlaceInfoRole: {
            return QVariant::fromValue(item->data);
        }
        default:
            break;
        }
    }

    return {};
}

void MapServersModel::addMarker(const PlaceInfo &place)
{
    TreeItem *countryItem = nullptr;

    // Find existing country
    for (auto &child : m_root->children) {
        if (child->name == place.country) {
            countryItem = child.get();
            break;
        }
    }

    // If not found, create new country node
    if (!countryItem) {
        auto newCountry = std::make_unique<TreeItem>();
        newCountry->name = place.country;
        newCountry->parent = m_root;

        countryItem = newCountry.get();

        int countryRow = static_cast<int>(m_root->children.size());

        LOG << "inserted group:" << newCountry->name;

        beginInsertRows(QModelIndex(), countryRow, countryRow);
        m_root->children.push_back(std::move(newCountry));
        endInsertRows();
    }

    // Add city under country
    int cityRow = static_cast<int>(countryItem->children.size());

    QModelIndex parentIndex = createIndex(countryItem->row(), 0, countryItem);

    auto newCity = std::make_unique<TreeItem>();
    newCity->name = place.town;
    newCity->data = place;
    newCity->parent = countryItem;

    LOG << "inserted item:" << newCity->data.country << newCity->data.town << newCity->data.location;

    beginInsertRows(parentIndex, cityRow, cityRow);
    countryItem->children.push_back(std::move(newCity));
    endInsertRows();
}

void MapServersModel::clear()
{
    beginResetModel();
    m_root->children.clear(); // unique_ptr handles recursive deletion
    endResetModel();
}
