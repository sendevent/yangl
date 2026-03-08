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

#include "pausecontroller.h"

#include "actions/actionstorage.h"
#include "app/common.h"
#include "app/nordvpninfo.h"
#include "app/statechecker.h"

#include <QApplication>
#include <QInputDialog>
#include <QTimer>
#include <utility>

PauseController::PauseController(ActionStorage *storage, StateChecker *checker, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_checker(checker)
    , m_pauseTimer(new QTimer(this))
{
    connect(m_pauseTimer, &QTimer::timeout, this, &PauseController::onPauseTimer);
    connect(m_checker, &StateChecker::statusChanged, this,
            [this](NordVpnInfo::Status status) { onStatusChanged(std::to_underlying(status)); });
}

bool PauseController::isPaused() const
{
    return m_pauseTimer->isActive();
}

void PauseController::pause(Action::NordVPN action)
{
    if (isPaused()) {
        return;
    }

    static const QHash<Action::NordVPN, int> durations {
        { Action::NordVPN::PauseCustom, 0 },
        { Action::NordVPN::Pause05, 5 },
        { Action::NordVPN::Pause30, 30 },
        { Action::NordVPN::Pause60, 60 },
    };

    int duration = durations.value(action, -1);
    if (-1 == duration) {
        WRN << "Unexpected pause type:" << std::to_underlying(action);
        return;
    }

    if (0 == duration) {
        bool ok(false);
        duration = QInputDialog::getInt({}, qApp->applicationDisplayName(), tr("Pause VPN for minutes:"), 1, 1, 1440, 1,
                                        &ok);
        if (!ok) {
            return;
        }
    }

    m_deadline = QDeadlineTimer(qint64(duration) * 60 * utils::oneSecondMs());

    if (auto disconnect = m_storage->action(Action::NordVPN::Disconnect)) {
        emit requestAction(disconnect.get());
        m_checker->startTransition();
        m_pauseTimer->start(utils::oneSecondMs());
    }
}

void PauseController::onPauseTimer()
{
    if (!m_deadline.hasExpired()) {
        static const qint64 preReconnectMs = 15 * 1000;
        if (m_deadline.remainingTime() <= preReconnectMs) {
            m_checker->startTransition();
        }
        return;
    }

    m_pauseTimer->stop();
    const NordVpnInfo &currentState = m_checker->state();
    if (currentState.status() == NordVpnInfo::Status::Unknown
        || currentState.status() == NordVpnInfo::Status::Disconnected) {
        if (auto connect = m_storage->action(Action::NordVPN::Connect)) {
            emit requestAction(connect.get());
        }
    }
}

void PauseController::onStatusChanged(int status)
{
    if (static_cast<NordVpnInfo::Status>(status) == NordVpnInfo::Status::Connected) {
        if (m_pauseTimer->isActive()) {
            m_pauseTimer->stop();
        }
    }
}
