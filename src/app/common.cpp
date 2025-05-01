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

namespace yangl {

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

    if (name == "Groups")
        return "group";

    return QString(name).replace(' ', '_');
}

QString nvpnToGeo(const QString &name)
{
    if (name.isEmpty())
        return "default";

    if (name == "group")
        return "Groups";

    return QString(name).replace('_', ' ');
}

} // namespace yangl
