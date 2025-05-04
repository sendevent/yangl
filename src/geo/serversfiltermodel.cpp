/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

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

#include "serversfiltermodel.h"

#include "app/common.h"
#include "geo/mapserversmodel.h"

ServersFilterModel::ServersFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setRecursiveFilteringEnabled(true);
}

bool ServersFilterModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    const auto &left = sourceLeft.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();
    const auto &right = sourceRight.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();

    const auto &leftData = sourceLeft.data(MapServersModel::Roles::PlaceInfoRole);
    const auto &rightData = sourceRight.data(MapServersModel::Roles::PlaceInfoRole);

    const bool leftIsGroup = left.country == utils::groupsTitle();
    const bool rightIsGroup = right.country == utils::groupsTitle();

    // Move "Groups" to the top
    if (leftIsGroup && !rightIsGroup) {
        return false;
    }
    if (!leftIsGroup && rightIsGroup) {
        return true;
    }

    // Default alphabetical sorting by country, then town
    return left.country < right.country && left.town < right.town;
}
