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

#include "appsettings.h"

#include "settingsmanager.h"
#include "statechecker.h"

#include <QSettings>
#include <QStandardPaths>

AppSetting::AppSetting(const QString &name, const QVariant &defaultValue)
    : Name(name)
    , DefaultValue(defaultValue)
{
}

QVariant AppSetting::read() const
{
    if (QSettings *settings = SettingsManager::instance()->storage())
        return settings->value(Name, DefaultValue);

    return QVariant();
}

void AppSetting::write(const QVariant &val) const
{

    if (QSettings *settings = SettingsManager::instance()->storage()) {
        settings->setValue(Name, val);
    }
}

/*static*/ void AppSetting::sync()
{
    if (QSettings *settings = SettingsManager::instance()->storage()) {
        settings->sync();
    }
}

OptionsGroup::OptionsGroup(const QString &name, const QVector<AppSetting *> &options,
                           const QVector<OptionsGroup *> &subroups)
    : Name(name)
    , Options(options)
    , Groups(subroups)
{
}

GroupMonitor::GroupMonitor()
    : OptionsGroup(localName(),
                   {
                           new AppSetting(QString("%1/NVPNPath").arg(localName()), QStringLiteral("/usr/bin/nordvpn")),
                           new AppSetting(QString("%1/Interval").arg(localName()), StateChecker::DefaultIntervalMs),
                           new AppSetting(QString("%1/MessageDuration").arg(localName()), 10),
                           new AppSetting(QString("%1/Active").arg(localName()), false),
                           new AppSetting(QString("%1/IgnoreFirstConnected").arg(localName()), true),
                           new AppSetting(QString("%1/EditorGeometry").arg(localName())),
                           new AppSetting(QString("%1/MsgPlainText").arg(localName()), false),
                   },
                   {})
{
}

GroupNVPN::GroupNVPN()
    : OptionsGroup(localName(), {}, {})
{
}

const GroupMonitor AppSettings::Monitor = {};
const GroupNVPN AppSettings::NordVPN = {};
