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

#include "flatplaceproxymodel.h"

#include "app/common.h"
#include "geo/mapserversmodel.h"

#include <qvariant.h>

FlatPlaceProxyModel::FlatPlaceProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

void FlatPlaceProxyModel::setSourceModel(QAbstractItemModel *model)
{
    beginResetModel();

    if (auto srcModel = sourceModel()) {
        disconnect(srcModel, nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(model);

    if (model) {
        connect(model, &QAbstractItemModel::rowsInserted, this, &FlatPlaceProxyModel::onRowsInserted);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &FlatPlaceProxyModel::onRowsRemoved);
        connect(model, &QAbstractItemModel::modelReset, this, &FlatPlaceProxyModel::rebuildFlatList);
    }

    rebuildFlatList();

    endResetModel();
}

bool isAcceptable(const auto &place)
{
    return place.ok && !place.isGroup() && !place.town.isEmpty() && place.location.isValid();
}

void FlatPlaceProxyModel::rebuildFlatList()
{
    m_places.clear();
    insertSubtree(QModelIndex());
}

void FlatPlaceProxyModel::insertSubtree(const QModelIndex &parent)
{
    const int rows = sourceModel()->rowCount(parent);
    for (int i = 0; i < rows; ++i) {
        const QModelIndex &child = sourceModel()->index(i, 0, parent);
        m_places.append(child);
        insertSubtree(child);
    }
}

int FlatPlaceProxyModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_places.size();
}

int FlatPlaceProxyModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QModelIndex FlatPlaceProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= m_places.size()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex FlatPlaceProxyModel::parent(const QModelIndex &) const
{
    return QModelIndex(); // Flat
}

QModelIndex FlatPlaceProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid() || proxyIndex.row() >= m_places.size()) {
        return QModelIndex();
    }

    const QModelIndex &src = m_places[proxyIndex.row()];
    return src.sibling(src.row(), proxyIndex.column());
}

QModelIndex FlatPlaceProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    const int row = m_places.indexOf(sourceIndex);
    return row != -1 ? createIndex(row, sourceIndex.column()) : QModelIndex();
}

void FlatPlaceProxyModel::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    // Recalculate and remove affected rows
    QVector<QModelIndex> toRemove;
    for (int row = first; row <= last; ++row) {
        QModelIndex child = sourceModel()->index(row, 0, parent);
        toRemove.append(child);
        QList<QModelIndex> stack = { child };
        while (!stack.isEmpty()) {
            QModelIndex current = stack.takeLast();
            toRemove.append(current);
            const int childCount = sourceModel()->rowCount(current);
            for (int i = 0; i < childCount; ++i)
                stack.append(sourceModel()->index(i, 0, current));
        }
    }

    for (const QModelIndex &idx : toRemove) {
        const int row = m_places.indexOf(idx);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_places.removeAt(row);
            endRemoveRows();
        }
    }
}

QVariant FlatPlaceProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < m_places.size()) {

        const auto &placeIndex = m_places.at(index.row());
        if (!placeIndex.isValid()) {
            WRN << "Invalid source index for row:" << index.row() << placeIndex;
            return {};
        }
        const auto &place = placeIndex.data(MapServersModel::PlaceInfoRole).value<PlaceInfo>();
        switch (role) {
        case Qt::DisplayRole: {
            return place.town;
        }
        case FlatPlaceProxyModel::Roles::PositionRole: {
            return QVariant::fromValue(place.location);
        }
        case FlatPlaceProxyModel::Roles::CountryNameRole: {
            return place.country;
        }
        case FlatPlaceProxyModel::Roles::CityNameRole: {
            return place.town;
        }
        case FlatPlaceProxyModel::Roles::PlaceInfoRole: {
            return QVariant::fromValue(place);
        }
        default:
            break;
        }
    }

    return {};
}

QHash<int, QByteArray> FlatPlaceProxyModel::roleNames() const
{
    return {
        { PositionRole, "position" },
        { CountryNameRole, "country" },
        { CityNameRole, "city" },
        { PlaceInfoRole, "placeInfo" },
    };
}

void FlatPlaceProxyModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{

    QList<QModelIndex> places;

    auto placeFromSource = [](const QModelIndex &index) -> PlaceInfo {
        if (!index.isValid()) {
            WRN << "Invalid index, ignored:" << index;
            return {};
        }

        return index.data(MapServersModel::PlaceInfoRole).value<PlaceInfo>();
    };

    const auto *model = parent.model() ? parent.model() : sourceModel();
    if (!model) {
        WRN << "No source model is set";
        return;
    }

    for (int row = first; row <= last; ++row) {
        const auto &child = model->index(row, 0, parent);
        if (isAcceptable(placeFromSource(child))) {
            places.append(child);
        }
    }

    if (places.size()) {
        beginInsertRows(QModelIndex(), m_places.size(), m_places.size() + places.size() - 1);
        m_places.append(places);
        endInsertRows();
    }
}
