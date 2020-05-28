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

#include "settingsmanager.h"

#include "common.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

SettingsManager *SettingsManager::m_instance = {};

/*static*/ QString SettingsManager::dirPath()
{
    static QString path;

    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir dir = QFileInfo(path).absoluteDir();
        if (!dir.exists())
            dir.mkpath(dir.absolutePath());
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
