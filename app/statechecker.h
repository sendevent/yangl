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

#include "ipccall.h"

#include <QObject>
#include <QQueue>

class IPCBus;
class QTimer;
class StateChecker : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        Unknown = 0,
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };
    Q_ENUM(State);

    explicit StateChecker(IPCBus *bus, QObject *parent = nullptr);

    void check();

    void setActive(bool active);
    bool isActive() const;

    void setInterval(int msecs);
    int inteval() const;

signals:
    void stateChanged(StateChecker::State state);

private slots:
    void onTimeout();
    void onQueryFinish(const QString &result);

protected:
    IPCBus *m_bus;
    QQueue<IPCCall::Ptr> m_calls;
    IPCCall::Ptr m_query;
    QTimer *m_timer;

    State m_state;
    State state() const;
    void setState(State state);

    void nextQuery();
    void updateState(const QString &from);
};
Q_DECLARE_METATYPE(StateChecker::State)
