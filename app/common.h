/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#pragma once

#include <QDateTime>
#include <QDebug>

#ifndef LOG
#define LOG qDebug() << yangl::now() << Q_FUNC_INFO
#endif // LOG

#ifndef WRN
#define WRN qWarning() << yangl::now() << Q_FUNC_INFO
#endif // WRN

#ifndef NIY
#define NIY WRN << "Not implemented yet"
#endif // NIY

namespace yangl {

static constexpr int OneSecondMs { 1000 };

static QString now()
{
    QDateTime d = QDateTime::currentDateTime();
    return d.toString("hh:mm:ss.zzz");
}

} // ns yangl
