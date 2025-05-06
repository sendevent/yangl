/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/action.h"
#include "nordvpninfo.h"

#include <QObject>

class CLICaller;
class QTimer;

class StateChecker : public QObject
{
    Q_OBJECT
public:
    static const int DefaultIntervalMs;

    using Ptr = QSharedPointer<StateChecker>;
    explicit StateChecker(CLICaller *bus, int intervalMs);
    ~StateChecker() override;

    void setCheckAction(const Action::Ptr &action);
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
    void onQueryFinish(const Action::Id &id, const QString &result, bool ok, const QString &info);

protected:
    CLICaller *m_bus;
    Action::Ptr m_actCheck;
    QTimer *m_timer;

    NordVpnInfo m_state;
    void setState(const NordVpnInfo &state);
    void setStatus(NordVpnInfo::Status status);

    void updateState(const QString &from);

    friend class TestStateChecker;
    friend class NordVpnWraper;
};
