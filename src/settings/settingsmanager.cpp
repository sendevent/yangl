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

#include "settingsmanager.h"

#include "app/common.h"

#include <QApplication>
#include <QSettings>
#include <QStandardPaths>

SettingsManager *SettingsManager::m_instance = {};

/*static*/ QString SettingsManager::dirPath()
{
    static QString path;

    if (path.isEmpty()) {
        path = utils::ensureDirExists(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        if (!path.endsWith(qAppName()))
            path = QStringLiteral("%1/%2").arg(path, qAppName());
    }

    return path;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QString("%1/settings.conf").arg(dirPath()), QSettings::IniFormat, this))
{
    LOG << "Config:" << m_settings->fileName();
}

SettingsManager *SettingsManager::instance()
{
    if (!m_instance)
        m_instance = new SettingsManager();
    return m_instance;
}

QSettings *SettingsManager::storage()
{
    return m_settings;
}

/*static*/ void SettingsManager::sync()
{
    instance()->storage()->sync();
}
