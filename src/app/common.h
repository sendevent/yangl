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

#pragma once

#include <QDateTime>
#include <QDebug>
#include <QGeoCoordinate>
#include <QMetaEnum>

#ifndef YANGL_TIMESTAMP
#define YANGL_TIMESTAMP QDateTime::currentDateTime().toString("t hh:mm:ss.zzz:")
#endif // YANGL_TIMESTAMP

#ifndef YANGL_LOG_PREFIX
#define YANGL_LOG_PREFIX qPrintable(YANGL_TIMESTAMP) << Q_FUNC_INFO
#endif // YANGL_LOG_PREFIX

#ifndef LOG
#define LOG qDebug() << YANGL_LOG_PREFIX
#endif // LOG

#ifndef WRN
#define WRN qWarning() << YANGL_LOG_PREFIX
#endif // WRN

#ifndef NIY
#define NIY WRN << "Not implemented yet"
#endif // NIY

namespace utils {
Q_NAMESPACE

inline int oneSecondMs()
{
    return 1000;
}

QString groupsTitle();

template<typename SomeQEnum>
QList<SomeQEnum> allEnum(const QList<SomeQEnum> &excluded = {})
{
    QList<SomeQEnum> values;
    const QMetaEnum &me = QMetaEnum::fromType<SomeQEnum>();
    for (int i = 0; i < me.keyCount(); ++i) {
        const SomeQEnum value = static_cast<SomeQEnum>(me.value(i));
        if (!excluded.contains(value))
            values << value;
    }

    return values;
}

QString ensureDirExists(const QString &path);

QString geoToNvpn(const QString &name);
QString nvpnToGeo(const QString &name);

std::tuple<QGeoCoordinate, bool> parseCoordinates(const QString &latStr, const QString &lonStr);
} // ns utils
