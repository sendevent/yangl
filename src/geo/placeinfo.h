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

#include <QGeoCoordinate>
#include <QObject>

struct PlaceInfo {

    QString country;
    QString town;
    QGeoCoordinate location;
    bool capital { false };
    bool ok { false };
    QString message;

    bool isGroup() const;

private:
    Q_GADGET
    Q_PROPERTY(QString country MEMBER country)
    Q_PROPERTY(QString town MEMBER town)
    Q_PROPERTY(QGeoCoordinate location MEMBER location);
};

using Places = QList<PlaceInfo>;
using CitiesByCountry = QMap<QString, QMultiMap<QString, PlaceInfo>>;
using RequestId = quint32;

Q_DECLARE_METATYPE(PlaceInfo)
