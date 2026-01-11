/*
Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#include "app/nordvpnwrapper.h"
#include "settings/appsettings.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QObject::tr("yangl"));
    a.setQuitOnLastWindowClosed(false);

    QLockFile lockFile(QDir::tempPath() + QStringLiteral("/yangl.lock"));
    if (!lockFile.tryLock(100))
        return 0;

    AppSettings::init();
    NordVpnWrapper::init();

    return a.exec();
}
