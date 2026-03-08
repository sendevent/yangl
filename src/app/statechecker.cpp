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

#include "statechecker.h"

#include "app/common.h"
#include "cli/clicaller.h"
#include "settings/appsettings.h"

#include <QTimer>

/*static*/ const int StateChecker::MaxConsecutiveErrors = 3;

StateChecker::StateChecker(CLICaller *bus, int intervalMs)
    : QObject()
    , m_bus(bus)
    , m_actCheck(nullptr)
    , m_timer(new QTimer(this))
    , m_pollInFlight(false)
    , m_consecutiveErrors(0)
    , m_uptimeTicker(new QTimer(this))
    , m_transitionTimer(new QTimer(this))
    , m_pollingMode(PollingMode::Dynamic)
    , m_customInterval(Duration(intervalMs))
    , m_state()
{
    connect(m_timer, &QTimer::timeout, this, &StateChecker::onTimeout);

    m_uptimeTicker->setInterval(
            static_cast<int>(std::chrono::duration_cast<Duration>(std::chrono::seconds(1)).count()));
    connect(m_uptimeTicker, &QTimer::timeout, this, &StateChecker::onUptimeTick);

    m_transitionTimer->setSingleShot(true);
    m_transitionTimer->setInterval(static_cast<int>(DynamicTransitionTimeout.count()));
    connect(m_transitionTimer, &QTimer::timeout, this, &StateChecker::endTransition);

    adjustDynamicInterval();

    setStatus(NordVpnInfo::Status::Unknown);
}

StateChecker::~StateChecker() { }

void StateChecker::setCheckAction(const Action::Ptr &action)
{
    if (m_actCheck != action) {
        if (m_actCheck) {
            disconnect(m_actCheck.get(), &Action::performed, this, &StateChecker::onQueryFinish);
        }

        m_actCheck = action;

        if (m_actCheck) {
            connect(m_actCheck.get(), &Action::performed, this, &StateChecker::onQueryFinish, Qt::UniqueConnection);
        }
    }
}

void StateChecker::setActive(bool active)
{
    if (m_timer->isActive() != active) {

        if (active) {
            check();
            m_timer->start();
        } else {
            stopTimer(); // sets Status::Unknown
        }

        AppSettings::Monitor->Active->write(active);
    }
}

bool StateChecker::isActive() const
{
    return m_timer->isActive();
}

void StateChecker::setInterval(int msecs)
{
    setInterval(Duration(msecs));
}

void StateChecker::setInterval(Duration interval)
{
    m_customInterval = interval;

    if (m_pollingMode == PollingMode::Custom) {
        const bool wasActive = isActive();
        m_timer->stop();
        m_timer->setInterval(static_cast<int>(m_customInterval.count()));
        if (wasActive) {
            m_timer->start();
        }
    }
}

void StateChecker::setPollingMode(PollingMode mode)
{
    if (m_pollingMode == mode) {
        return;
    }

    m_pollingMode = mode;
    m_transitionTimer->stop();

    if (m_pollingMode == PollingMode::Dynamic) {
        adjustDynamicInterval();
    } else {
        const bool wasActive = isActive();
        m_timer->stop();
        m_timer->setInterval(static_cast<int>(m_customInterval.count()));
        if (wasActive) {
            m_timer->start();
        }
    }
}

int StateChecker::interval() const
{
    return static_cast<int>(m_customInterval.count());
}

void StateChecker::check()
{
    if (m_pollInFlight) {
        return;
    }

    if (!m_actCheck) {
        notifyError(tr("Invalid Status-check Action instance"));
    } else if (!m_bus->runCall(m_actCheck->createRequest())) {
        notifyError(tr("Failed to dispatch status check"));
    } else {
        m_pollInFlight = true;
    }
}

void StateChecker::onQueryFinish(const Action::Id &id, const QString &result, bool ok, const Action::RunInfo &info)
{
    m_pollInFlight = false;

    if (ok) {
        m_consecutiveErrors = 0;
        updateState(result);
    } else {
        QString message = tr("Action invocation `%1` failed:").arg(id.toString());
        if (!info.errors.isEmpty()) {
            const auto &lineSeparator =
                    QLatin1String(AppSettings::Tray->MessagePlainText->read().toBool() ? "\n" : "<br>");
            message.append(lineSeparator);
            message.append(utils::composeMessage(info));
        }
        notifyError(message);
    }
}

void StateChecker::onTimeout()
{
    check();
}

void StateChecker::updateState(const QString &from)
{
    setState(NordVpnInfo::fromString(from));
}

NordVpnInfo StateChecker::state() const
{
    return m_state;
}

void StateChecker::setState(const NordVpnInfo &state)
{
    if (this->state() != state) {
        const bool statusDetailsChanged = m_state.status() != state.status() || m_state.country() != state.country()
                || m_state.city() != state.city();

        m_state = state;

        if (m_pollingMode == PollingMode::Dynamic && statusDetailsChanged) {
            m_transitionTimer->start();
            adjustDynamicInterval();
        }

        if (state.status() == NordVpnInfo::Status::Connected) {
            if (!m_uptimeTicker->isActive()) {
                m_uptimeTicker->start();
            }
        } else {
            m_uptimeTicker->stop();
        }

        if (statusDetailsChanged) {
            emit statusChanged(state.status());
        }
        emit stateChanged(m_state);
    }
}

void StateChecker::setStatus(NordVpnInfo::Status status)
{
    if (m_state.status() != status) {
        NordVpnInfo state;
        if (status != NordVpnInfo::Status::Unknown) {
            state = m_state;
        }

        state.setStatus(status);
        setState(state);
    }
}

void StateChecker::notifyError(const QString &errorMessage)
{
    WRN << errorMessage;
    emit error(errorMessage);

    if (++m_consecutiveErrors >= MaxConsecutiveErrors) {
        WRN << "Stopping monitor after" << m_consecutiveErrors << "consecutive errors";
        m_consecutiveErrors = 0;
        stopTimer(); // sets Status::Unknown
    }
}

void StateChecker::stopTimer()
{
    m_timer->stop();
    m_uptimeTicker->stop();
    setStatus(NordVpnInfo::Status::Unknown);
}

void StateChecker::onUptimeTick()
{
    m_state.tickUptime();
    emit stateChanged(m_state);
}

void StateChecker::startTransition()
{
    if (m_pollingMode != PollingMode::Dynamic) {
        return;
    }

    m_transitionTimer->start();
    adjustDynamicInterval();
}

void StateChecker::endTransition()
{
    m_transitionTimer->stop();
    adjustDynamicInterval();
}

void StateChecker::adjustDynamicInterval()
{
    const Duration newInterval = m_transitionTimer->isActive() ? DynamicIntervalTransitional : DynamicIntervalStable;

    if (m_timer->interval() == static_cast<int>(newInterval.count())) {
        return;
    }

    const bool wasActive = isActive();
    m_timer->stop();
    m_timer->setInterval(static_cast<int>(newInterval.count()));
    if (wasActive) {
        m_timer->start();
    }
}
