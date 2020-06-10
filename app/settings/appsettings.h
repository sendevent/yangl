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

#pragma once

#include <QString>
#include <QVariant>
#include <QVector>

class AppSetting
{
public:
    AppSetting(const QString &name, const QVariant &defaultValue = QVariant());
    const QString Name;
    const QVariant DefaultValue;

    QVariant read() const;
    void write(const QVariant &val) const;

private:
    AppSetting() = delete;
    AppSetting(const AppSetting &) = delete;
    AppSetting &operator=(const AppSetting &) = delete;
};

class OptionsGroup
{
public:
    OptionsGroup(const QString &name, const QVector<AppSetting *> &options, const QVector<OptionsGroup *> &subroups);

    const QString Name;
    const QVector<AppSetting *> Options;
    const QVector<OptionsGroup *> Groups;

private:
    OptionsGroup() = delete;
    OptionsGroup(const OptionsGroup &) = delete;
    OptionsGroup &operator=(const OptionsGroup &) = delete;
};

class GroupMonitor : public OptionsGroup
{
public:
    GroupMonitor();

    const AppSetting *NVPNPath = Options[0];
    const AppSetting *Interval = Options[1];
    const AppSetting *Active = Options[2];
    const AppSetting *SettingsDialog = Options[3];
    const AppSetting *LogLinesLimit = Options[4];

private:
    GroupMonitor(const GroupMonitor &) = delete;
    GroupMonitor &operator=(const GroupMonitor &) = delete;

    static const QString localName() { return "Monitor"; }
};

class GroupMap : public OptionsGroup
{
public:
    GroupMap();

    const AppSetting *Geometry = Options[0];
    const AppSetting *Filter = Options[1];
    const AppSetting *Visible = Options[2];
    const AppSetting *CenterLat = Options[3];
    const AppSetting *CenterLon = Options[4];
    const AppSetting *Scale = Options[5];
    const AppSetting *MapPlugin = Options[6];
    const AppSetting *MapType = Options[7];

private:
    GroupMap(const GroupMap &) = delete;
    GroupMap &operator=(const GroupMap &) = delete;

    static const QString localName() { return "Map"; }
};

class GroupTray : public OptionsGroup
{
public:
    GroupTray();

    const AppSetting *MessageDuration = Options[0];
    const AppSetting *IgnoreFirstConnected = Options[1];
    const AppSetting *MessagePlainText = Options[2];
    const AppSetting *IcnUnknown = Options[3];
    const AppSetting *IcnUnknownSub = Options[4];
    const AppSetting *IcnDisconnected = Options[5];
    const AppSetting *IcnDisconnectedSub = Options[6];
    const AppSetting *IcnConnecting = Options[7];
    const AppSetting *IcnConnectingSub = Options[8];
    const AppSetting *IcnConnected = Options[9];
    const AppSetting *IcnConnectedSub = Options[10];

private:
    GroupTray(const GroupTray &) = delete;
    GroupTray &operator=(const GroupTray &) = delete;

    static const QString localName() { return "Tray"; }
    static QString iconPath(const QString &iconFile);
};

class AppSettings
{
public:
    static GroupMonitor *Monitor;
    static GroupMap *Map;
    static GroupTray *Tray;

    static void init();

    static void sync();

private:
    AppSettings() = delete;
    AppSettings(const AppSetting &) = delete;
    AppSettings &operator=(const AppSettings &) = delete;
};
