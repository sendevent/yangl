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

#include "appsettings.h"

#include "actions/clicallresultview.h"
#include "app/common.h"
#include "app/statechecker.h"
#include "geo/mapwidget.h"
#include "settings/settingsmanager.h"

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

OptionsGroup::OptionsGroup(const QString &name, const QList<AppSetting *> &options,
                           const QList<OptionsGroup *> &subroups)
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
                           new AppSetting(QString("%1/Active").arg(localName()), false),
                           new AppSetting(QString("%1/EditorGeometry").arg(localName())),
                           new AppSetting(QString("%1/LogLinesLimit").arg(localName()),
                                          CLICallResultView::MaxBlocksCountDefault),
                   },
                   {})
{
}

GroupMap::GroupMap()
    : OptionsGroup(localName(),
                   {
                           new AppSetting(QString("%1/Geometry").arg(localName()), {}),
                           new AppSetting(QString("%1/Filter").arg(localName()), QString()),
                           new AppSetting(QString("%1/Visible").arg(localName()), false),
                           new AppSetting(QString("%1/CenterLat").arg(localName()), {}),
                           new AppSetting(QString("%1/CenterLon").arg(localName()), {}),
                           new AppSetting(QString("%1/Scale").arg(localName()), 2.5),
                           new AppSetting(QString("%1/Plugin").arg(localName()),
                                          []() {
                                              const auto &plugins = MapWidget::geoServices();
                                              return plugins.size() ? plugins.first() : "osm";
                                          }()),
                           new AppSetting(QString("%1/Type").arg(localName()), 6),
                   },
                   {})
{
}

GroupTray::GroupTray()
    : OptionsGroup(localName(),
                   {
                           new AppSetting(QString("%1/MessageDuration").arg(localName()), 10),
                           new AppSetting(QString("%1/IgnoreFirstConnected").arg(localName()), true),
                           new AppSetting(QString("%1/MsgPlainText").arg(localName()), false),

                           new AppSetting(QString("%1/IcnUnknown").arg(localName()),
                                          iconPath(QStringLiteral("unknown.png"))),
                           new AppSetting(QString("%1/IcnUnknownSub").arg(localName()),
                                          iconPath(QStringLiteral("unknown_sub.png"))),
                           new AppSetting(QString("%1/IcnDisconnected").arg(localName()),
                                          iconPath(QStringLiteral("disconnected.png"))),
                           new AppSetting(QString("%1/IcnDisconnectedSub").arg(localName()),
                                          iconPath(QStringLiteral("disconnected_sub.png"))),
                           new AppSetting(QString("%1/IcnConnecting").arg(localName()),
                                          iconPath(QStringLiteral("connecting.png"))),
                           new AppSetting(QString("%1/IcnConnectingSub").arg(localName()),
                                          iconPath(QStringLiteral("connecting_sub.png"))),
                           new AppSetting(QString("%1/IcnConnected").arg(localName()),
                                          iconPath(QStringLiteral("connected.png"))),
                           new AppSetting(QString("%1/IcnConnectedSub").arg(localName()),
                                          iconPath(QStringLiteral("connected_sub.png"))),
                   },
                   {})
{
}

/*static*/ QString GroupTray::iconPath(const QString &iconFile)
{
    return yangl::ensureDirExists(QString("%1/tray/%2").arg(SettingsManager::dirPath(), iconFile));
}

/*static*/ void AppSettings::sync()
{
    if (QSettings *settings = SettingsManager::instance()->storage()) {
        settings->sync();
    }
}

/*static*/ GroupMonitor *AppSettings::Monitor = {};
/*static*/ GroupMap *AppSettings::Map = {};
/*static*/ GroupTray *AppSettings::Tray = {};

/*static*/ void AppSettings::init()
{
#ifdef QT_TESTLIB_LIB
    QStandardPaths::setTestModeEnabled(true);
#endif

    Monitor = new GroupMonitor;
    Map = new GroupMap;
    Tray = new GroupTray;
}
