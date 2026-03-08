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
#include <chrono>
#include <utility>

PauseController::PauseController(ActionStorage *storage, StateChecker *checker, QObject *parent)
    : QObject(parent)
    , m_storage(storage)
    , m_checker(checker)
    , m_pauseTimer(new QTimer(this))
{
    connect(m_pauseTimer, &QTimer::timeout, this, &PauseController::onPauseTimer);
    connect(m_checker, &StateChecker::statusChanged, this, &PauseController::onStatusChanged);
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

    std::chrono::minutes duration { 0 };
    switch (action) {
    case Action::NordVPN::PauseCustom:
        break;
    case Action::NordVPN::Pause05:
        duration = std::chrono::minutes(5);
        break;
    case Action::NordVPN::Pause30:
        duration = std::chrono::minutes(30);
        break;
    case Action::NordVPN::Pause60:
        duration = std::chrono::minutes(60);
        break;
    default:
        WRN << "Unexpected pause type:" << std::to_underlying(action);
        return;
    }

    if (duration == std::chrono::minutes(0)) {
        bool ok(false);
        const int minutes =
                QInputDialog::getInt({}, qApp->applicationDisplayName(), tr("Pause VPN for minutes:"), 1, 1, 1440, 1, &ok);
        if (!ok) {
            return;
        }
        duration = std::chrono::minutes(minutes);
    }

    m_deadline = QDeadlineTimer(
            static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));

    if (auto disconnect = m_storage->action(Action::NordVPN::Disconnect)) {
        emit requestAction(disconnect.get());
        m_checker->startTransition();
        m_pauseTimer->start(static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                     std::chrono::seconds(1))
                                                     .count()));
    }
}

void PauseController::onPauseTimer()
{
    if (!m_deadline.hasExpired()) {
        const auto preReconnect = std::chrono::seconds(15);
        if (m_deadline.remainingTime()
            <= static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(preReconnect).count())) {
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

void PauseController::onStatusChanged(NordVpnInfo::Status status)
{
    if (status == NordVpnInfo::Status::Connected) {
        if (m_pauseTimer->isActive()) {
            m_pauseTimer->stop();
        }
    }
}
