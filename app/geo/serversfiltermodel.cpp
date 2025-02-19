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

#include "serversfiltermodel.h"

#include "app/common.h"

ServersFilterModel::ServersFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ServersFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QRegExp &filter = filterRegExp();
    auto isAcceptable = [&filter](const QModelIndex &id) { return id.data().toString().contains(filter); };
    const QModelIndex &current = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceParent.isValid())
        for (int i = 0; i < sourceModel()->rowCount(current); ++i)
            if (isAcceptable(sourceModel()->index(i, 0, current)))
                return true;

    return isAcceptable(current) || isAcceptable(sourceParent);
}
