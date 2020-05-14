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

#include <QDebug>
#include <QMetaEnum>

#define LOG qDebug() << Q_FUNC_INFO

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(iconForState(StateChecker::Disconnected), parent)
{
}

/*static*/ QIcon TrayIcon::iconForState(StateChecker::State state)
{
    static const QMap<StateChecker::State, QIcon> staticIcons = [] {
        QMap<StateChecker::State, QIcon> icons;
        QMetaEnum me = QMetaEnum::fromType<StateChecker::State>();
        for (int i = 0; i < me.keyCount(); ++i) {
            const StateChecker::State state = static_cast<StateChecker::State>(me.value(i));
            switch (state) {
            case StateChecker::State::Connected: {
                icons.insert(state, QPixmap(":/icn/resources/online.png"));
                break;
            }
            case StateChecker::State::Disconnected:
            case StateChecker::State::Connecting:
            case StateChecker::State::Disconnecting:
            default: {
                icons.insert(state, QPixmap(":/icn/resources/offline.png"));
                break;
            }
            }
        }
        return icons;
    }();

    return staticIcons[state];
}

void TrayIcon::setState(StateChecker::State state)
{
    LOG << state;
    setIcon(iconForState(state));
}
