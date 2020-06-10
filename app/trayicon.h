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

#include "statechecker.h"

#include <QSystemTrayIcon>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    TrayIcon(QObject *parent = {});

    static void reloadIcons();

    void setMessageDuration(int durationSecs);
    int duration() const { return m_duration; }

    void updateIcon(NordVpnInfo::Status status);

public slots:
    void setState(const NordVpnInfo &state);

private:
    struct IconInfo {
        QString m_base;
        QString m_sub;
        NordVpnInfo::Status m_status;
    };
    static QMap<NordVpnInfo::Status, IconInfo> m_allIcons;
    static QMap<NordVpnInfo::Status, QIcon> m_composedIcons;

    NordVpnInfo m_state;
    bool m_isFirstChange;
    int m_duration;

    static QIcon iconForState(const NordVpnInfo &state);
    static QIcon iconForStatus(const NordVpnInfo::Status &status);

    static TrayIcon::IconInfo infoPixmaps(const NordVpnInfo::Status forStatus);
    static QIcon generateIcon(const NordVpnInfo::Status forStatus);

    void deployDefaults() const;
};
