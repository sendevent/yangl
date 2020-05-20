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

#include "trayicon.h"

#include "appsettings.h"

#include <QApplication>
#include <QDebug>
#include <QMetaEnum>

#define LOG qDebug() << Q_FUNC_INFO

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(iconForStatus(NordVpnInfo::Status::Disconnected), parent)
    , m_isFirstChange(true)
{
}

/*static*/ QIcon TrayIcon::iconForState(const NordVpnInfo &state)
{
    return iconForStatus(state.status());
}

/*static*/ QIcon TrayIcon::iconForStatus(const NordVpnInfo::Status &status)
{
    static const QMap<NordVpnInfo::Status, QIcon> staticIcons = [] {
        QMap<NordVpnInfo::Status, QIcon> icons;
        QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
        for (int i = 0; i < me.keyCount(); ++i) {
            const NordVpnInfo::Status state = static_cast<NordVpnInfo::Status>(me.value(i));
            switch (state) {
            case NordVpnInfo::Status::Connected: {
                icons.insert(state, QPixmap(":/icn/resources/online.png"));
                break;
            }
            case NordVpnInfo::Status::Disconnected:
            case NordVpnInfo::Status::Connecting:
            case NordVpnInfo::Status::Disconnecting:
            default: {
                icons.insert(state, QPixmap(":/icn/resources/offline.png"));
                break;
            }
            }
        }
        return icons;
    }();

    return staticIcons[status];
}

void TrayIcon::setMessageDuration(int durationSecs)
{
    m_duration = durationSecs;
}

void TrayIcon::setState(const NordVpnInfo &state)
{
    const QString description = state.toString();

    if (m_state.status() != state.status() && !qApp->isSavingSession()) {
        QIcon icn = iconForStatus(state.status());
        setIcon(icn);

        bool skeepMessage(false);
        if (m_isFirstChange && state.status() == NordVpnInfo::Status::Connected)
            if (AppSettings::Monitor.Active->read().toBool()
                && AppSettings::Monitor.IgnoreFirstConnected->read().toBool())
                skeepMessage = true;

        if (!skeepMessage)
            showMessage(qApp->applicationDisplayName(), description, {}, m_duration);
    }

    setToolTip(description);

    m_state = state;
    m_isFirstChange = false;
}
