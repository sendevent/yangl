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

#include <qnamespace.h>

FlatPlaceProxyModel::FlatPlaceProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

void FlatPlaceProxyModel::setSourceModel(QAbstractItemModel *source)
{

    if (auto srcModel = sourceModel()) {
        disconnect(srcModel, &QAbstractItemModel::rowsInserted, this, &FlatPlaceProxyModel::onRowsInserted);
    }

    QIdentityProxyModel::setSourceModel(source);

    if (auto srcModel = source) {
        connect(srcModel, &QAbstractItemModel::rowsInserted, this, &FlatPlaceProxyModel::onRowsInserted);
    }

    rebuildFlatList();
}

bool isAcceptable(const auto &place)
{
    return /*place.ok &&*/ !place.isGroup() && !place.town.isEmpty() && place.location.isValid();
};

void FlatPlaceProxyModel::rebuildFlatList()
{
    beginResetModel();

    m_places.clear();

    if (auto *m_source = sourceModel()) {
        for (int topLevelRow = 0; topLevelRow < m_source->rowCount(); ++topLevelRow) {
            const auto &topLevelIndex = m_source->index(topLevelRow, 0);
            for (int subRow = 0; subRow < m_source->rowCount(topLevelIndex); ++subRow) {
                const auto &subIndex = m_source->index(subRow, 0, topLevelIndex);
                const auto &place = subIndex.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();
                if (isAcceptable(place)) {
                    m_places.append(place);
                }
            }
        }
    }

    endResetModel();
}

int FlatPlaceProxyModel::rowCount(const QModelIndex &) const
{
    return m_places.size();
}

QVariant FlatPlaceProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < m_places.size()) {

        const auto &place = m_places.at(index.row());
        switch (role) {
        case Qt::DisplayRole: {
            return place.town;
        }
        case PositionRole: {
            return QVariant::fromValue(place.location);
        }
        case CountryNameRole: {
            return place.country;
        }
        case CityNameRole: {
            return place.town;
        }
        default:
            break;
        }
    }

    return QIdentityProxyModel::data(index, role);
}

QHash<int, QByteArray> FlatPlaceProxyModel::roleNames() const
{
    return {
        { PositionRole, "position" },
        { CountryNameRole, "country" },
        { CityNameRole, "city" },
    };
}

void FlatPlaceProxyModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{

    QList<PlaceInfo> places;

    if (auto *srcModel = sourceModel()) {
        for (int row = first; row <= last; ++row) {
            const auto &id = srcModel->index(row, 0, parent);
            const auto &place = id.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();
            if (isAcceptable(place)) {
                places.append(place);
                LOG << row << place.country << place.town;
            }
        }
    }

    if (const int newCount = places.size()) {
        const int currentCount = rowCount();
        LOG << currentCount << newCount;
        beginInsertRows(QModelIndex(), currentCount, currentCount + newCount - 1);

        m_places.append(places);

        endInsertRows();
    }
}
