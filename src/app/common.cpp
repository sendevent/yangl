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

#include "common.h"

#include "geo/placeinfo.h"
#include "settings/appsettings.h"
#include "version/appversiondefs.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLatin1StringView>

namespace utils {

QString groupsTitle()
{
    return geo::groupsTitle();
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

bool isValidAppPath(const QString &path, QString *reason)
{
    if (path.isEmpty()) {
        const QString &msg = QObject::tr("Target binary path is empty");
        WRN << msg;
        if (reason) {
            *reason = msg;
        }
        return false;
    }

    const QFileInfo info(path);
    if (!info.exists()) {
        const QString &msg = QObject::tr("Target binary file not exists: <br><b>`%1`</b>").arg(path);
        WRN << msg;
        if (reason) {
            *reason = msg;
        }
        return false;
    }

    if (!info.isExecutable()) {
        const QString &msg = QObject::tr("Target binary file file is not executable: `%1`").arg(path);
        WRN << msg;
        if (reason) {
            *reason = msg;
        }
        return false;
    }

    if (reason) {
        *reason = {};
    }

    return true;
}

QString composeTitle(const QString &payload)
{
    return QObject::tr("%1 %2 — %3").arg(qApp->applicationName(), yangl::V.trio(), payload);
}

QString composeMessage(const Action::RunInfo &actionInfo)
{
    static const QLatin1String br("<br/>");
    static const QLatin1String nl("\n");

    const bool usePlainText = AppSettings::Tray->MessagePlainText->read().toBool();
    const QLatin1String lineSeparator = usePlainText ? nl : br;

    static const QString tmplHtml("<b>%1:</b>");
    static const QString tmplPlainText("%1: ");
    auto wrappPart = [&usePlainText](const QString &value) {
        return QString(usePlainText ? tmplPlainText : tmplHtml).arg(value);
    };

    const QMap<QString, QString> parts {
        { QObject::tr("Result"), actionInfo.result },
        { QObject::tr("Exit code"), actionInfo.exitCode },
        { QObject::tr("Errors"), actionInfo.errors },
    };

    QString message = QString("%1 ").arg(actionInfo.timeStamp);
    for (const auto [key, value] : parts.asKeyValueRange()) {

        const auto &scopeLines = value.split('\n');
        const auto &line = scopeLines.join(lineSeparator);
        if (!line.isEmpty()) {
            message.append(wrappPart(key));

            if (scopeLines.size() > 1) {
                message.append(lineSeparator);
            }

            message.append(QString("%1%2").arg(line, lineSeparator));
        }
    }

    message = QString("%1:%2%3").arg(qApp->applicationDisplayName(), lineSeparator, message);

    return message;
}

} // namespace utils
