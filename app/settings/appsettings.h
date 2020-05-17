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

    static void sync();

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

    const AppSetting *SettingsDialog = Options[0];
    const AppSetting *NVPNPath = Options[1];
    const AppSetting *Active = Options[2];
    const AppSetting *Interval = Options[3];

private:
    GroupMonitor(const GroupMonitor &) = delete;
    GroupMonitor &operator=(const GroupMonitor &) = delete;

    static const QString localName() { return "Monitor"; }
};

class GroupNVPN : public OptionsGroup
{
public:
    GroupNVPN();

private:
    GroupNVPN(const GroupNVPN &) = delete;
    GroupNVPN &operator=(const GroupNVPN &) = delete;

    static const QString localName() { return "NordVpn"; }
};

class AppSettings
{
public:
    static const GroupMonitor Monitor;
    static const GroupNVPN NordVPN;

private:
    AppSettings() = delete;
    AppSettings(const AppSetting &) = delete;
    AppSettings &operator=(const AppSettings &) = delete;
};
