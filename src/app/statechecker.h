/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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
#include "app/nordvpninfo.h"

#include <QObject>

class CLICaller;
class QTimer;

class StateChecker : public QObject
{
    Q_OBJECT
public:
    enum class PollingMode
    {
        Dynamic,
        Custom,
    };
    Q_ENUM(PollingMode)

    static const int DefaultIntervalMs;
    static const int MaxConsecutiveErrors;
    static const int DynamicIntervalTransitionalMs;
    static const int DynamicIntervalStableMs;
    static const int DynamicTransitionTimeoutMs;

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
    void setPollingMode(PollingMode mode);

signals:
    void stateChanged(const NordVpnInfo &state);
    void statusChanged(const NordVpnInfo::Status status);
    void error(const QString &message);

private slots:
    void onTimeout();
    void onQueryFinish(const Action::Id &id, const QString &result, bool ok, const Action::RunInfo &info);
    void onUptimeTick();

protected:
    CLICaller *m_bus;
    Action::Ptr m_actCheck;
    QTimer *m_timer;
    bool m_pollInFlight;
    int m_consecutiveErrors;
    QTimer *m_uptimeTicker;
    QTimer *m_transitionTimer;
    PollingMode m_pollingMode;
    int m_customIntervalMs;

    NordVpnInfo m_state;
    void setState(const NordVpnInfo &state);
    void setStatus(NordVpnInfo::Status status);

    void updateState(const QString &from);

    void notifyError(const QString &errorMessage);

    void stopTimer();
    void endTransition();
    void adjustDynamicInterval();

    friend class TestStateChecker;
    friend class NordVpnWrapper;
};
