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

#include "version.h"

namespace yangl {

QString VersionInfo::trio() const
{
    static const QString v = QStringLiteral("%1.%2.%3").arg(Major).arg(Minor).arg(Patch);
    return v;
}

QString VersionInfo::commit() const
{
    static const QString v = QStringLiteral("%1@%2%3").arg(Branch).arg(Commit).arg(BranchDirty ? QStringLiteral("*")
                                                                                               : QStringLiteral(""));
    return v;
}

} // namespace yangl
