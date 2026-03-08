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

#include <chrono>
#include <QObject>

class CLICaller;
class QTimer;

/*!
 * \brief Periodic VPN state monitor.
 *
 * Fires a \c CLICaller request on a configurable timer and parses the result
 * into a \c NordVpnInfo. Two polling modes are supported:
 *
 * \li \b Dynamic — polls every \c DynamicIntervalTransitionalMs (1 s) while
 *     the connection is transitioning and backs off to \c DynamicIntervalStableMs
 *     (5 s) once the state has been stable for \c DynamicTransitionTimeoutMs.
 *     Between real polls the uptime counter is ticked locally at 1 s.
 * \li \b Custom — polls at a fixed user-defined interval regardless of state.
 *
 * An in-flight guard (\c m_pollInFlight) prevents overlapping requests.
 * After \c MaxConsecutiveErrors successive failures the monitor stops and
 * emits \c error; a successful poll resets the counter.
 */
class StateChecker : public QObject
{
    Q_OBJECT
public:
    using Duration = std::chrono::milliseconds;

    enum class PollingMode
    {
        Dynamic,
        Custom,
    };
    Q_ENUM(PollingMode)

    static constexpr Duration DefaultInterval = std::chrono::seconds(10);
    static constexpr int DefaultIntervalMs = static_cast<int>(DefaultInterval.count());
    static const int MaxConsecutiveErrors;
    static constexpr Duration DynamicIntervalTransitional = std::chrono::seconds(1);
    static constexpr int DynamicIntervalTransitionalMs = static_cast<int>(DynamicIntervalTransitional.count());
    static constexpr Duration DynamicIntervalStable = std::chrono::seconds(10);
    static constexpr int DynamicIntervalStableMs = static_cast<int>(DynamicIntervalStable.count());
    static constexpr Duration DynamicTransitionTimeout = std::chrono::seconds(30);
    static constexpr int DynamicTransitionTimeoutMs = static_cast<int>(DynamicTransitionTimeout.count());

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
    void setInterval(Duration interval);
    void setActive(bool active);
    void setPollingMode(PollingMode mode);
    void startTransition();

signals:
    void stateChanged(const NordVpnInfo &state);
    void statusChanged(const NordVpnInfo::Status status);
    void error(const QString &message);

private slots:
    void onTimeout();
    void onQueryFinish(const Action::Id &id, const QString &result, bool ok, const Action::RunInfo &info);
    void onUptimeTick();

protected:
    CLICaller *m_bus { nullptr };
    Action::Ptr m_actCheck;
    QTimer *m_timer { nullptr };
    bool m_pollInFlight { false };
    int m_consecutiveErrors { 0 };
    QTimer *m_uptimeTicker { nullptr };
    QTimer *m_transitionTimer { nullptr };
    PollingMode m_pollingMode { PollingMode::Dynamic };
    Duration m_customInterval { 0 };

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
