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

#include "action.h"
#include "nordvpninfo.h"

#include <QObject>
#include <QQueue>

class CLICaller;
class ActionStorage;
class QTimer;
class StateChecker : public QObject
{
    Q_OBJECT
public:
    static constexpr int DefaultIntervalMs = 1000;

    explicit StateChecker(CLICaller *bus, ActionStorage *actions, int intervalMs, QObject *parent = nullptr);
    ~StateChecker() override;

    void check();

    bool isActive() const;
    int interval() const;
    NordVpnInfo state() const;

public slots:
    void setInterval(int msecs);
    void setActive(bool active);

signals:
    void stateChanged(const NordVpnInfo &state);
    void statusChanged(const NordVpnInfo::Status status);

private slots:
    void onTimeout();
    void onQueryFinish(const QString &result, bool ok);

protected:
    CLICaller *m_bus;
    ActionStorage *m_actions;
    QQueue<Action::Ptr> m_calls;
    Action::Ptr m_currAction;
    QTimer *m_timer;

    NordVpnInfo m_state;
    void setState(const NordVpnInfo &state);
    void setStatus(NordVpnInfo::Status status);

    void nextQuery();
    void updateState(const QString &from);

    friend class tst_StateChecker;
    friend class NordVpnWraper;
};
