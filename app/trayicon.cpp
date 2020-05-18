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
    : QSystemTrayIcon(iconForStatus(StateChecker::Status::Disconnected), parent)
    , m_isFirstChange(true)
{
}

/*static*/ QIcon TrayIcon::iconForState(const StateChecker::Info &state)
{
    return iconForStatus(state.m_status);
}

/*static*/ QIcon TrayIcon::iconForStatus(const StateChecker::Status &status)
{
    static const QMap<StateChecker::Status, QIcon> staticIcons = [] {
        QMap<StateChecker::Status, QIcon> icons;
        QMetaEnum me = QMetaEnum::fromType<StateChecker::Status>();
        for (int i = 0; i < me.keyCount(); ++i) {
            const StateChecker::Status state = static_cast<StateChecker::Status>(me.value(i));
            switch (state) {
            case StateChecker::Status::Connected: {
                icons.insert(state, QPixmap(":/icn/resources/online.png"));
                break;
            }
            case StateChecker::Status::Disconnected:
            case StateChecker::Status::Connecting:
            case StateChecker::Status::Disconnecting:
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

void TrayIcon::setState(const StateChecker::Info &state)
{
    const QString description = state.toString();

    if (m_state.m_status != state.m_status && !qApp->isSavingSession()) {
        QIcon icn = iconForStatus(state.m_status);
        setIcon(icn);

        bool skeepMessage(false);
        if (m_isFirstChange && state.m_status == StateChecker::Status::Connected)
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
