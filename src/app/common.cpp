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

#include "common.h"

#include <QDir>
#include <QFileInfo>

namespace utils {

QString groupsTitle()
{
    static const QString &title = QObject::tr("Groups");
    return title;
}

QString ensureDirExists(const QString &path)
{
    if (path.isEmpty()) {
        WRN << "Empty path";
        return path;
    }

    const QFileInfo info(path);
    const QDir &dir = info.absoluteDir();
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());

    const QString res = info.absoluteFilePath();
    return res;
}

QString geoToNvpn(const QString &name)
{
    if (name == "default")
        return {};

    return QString(name).replace(' ', '_');
}

QString nvpnToGeo(const QString &name)
{
    if (name.isEmpty())
        return "default";

    return QString(name).replace('_', ' ');
}

std::tuple<QGeoCoordinate, bool> parseCoordinates(const QString &latStr, const QString &lonStr)
{
    bool parsed(false);
    QGeoCoordinate coordinate;

    if (!latStr.isEmpty() && !lonStr.isEmpty()) {

        const auto lat = latStr.toDouble(&parsed);
        if (parsed) {
            const auto lon = lonStr.toDouble(&parsed);
            if (parsed) {
                coordinate = QGeoCoordinate(lat, lon);
            }
        }
    }

    return { coordinate, parsed };
};
} // namespace utils
