/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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

#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>

/*static*/ const int StateChecker::DefaultIntervalMs = yangl::OneSecondMs;

StateChecker::StateChecker(CLICaller *bus, int intervalMs)
    : QObject()
    , m_bus(bus)
    , m_actCheck(nullptr)
    , m_timer(new QTimer(this))
    , m_state()
{
    setInterval(intervalMs);
    connect(m_timer, &QTimer::timeout, this, &StateChecker::onTimeout);

    setStatus(NordVpnInfo::Status::Unknown);
}

StateChecker::~StateChecker() { }

void StateChecker::setCheckAction(const Action::Ptr &action)
{
    if (m_actCheck != action) {
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
            m_timer->stop();
            setStatus(NordVpnInfo::Status::Unknown);
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
    const bool wasActive(isActive());

    m_timer->stop();
    m_timer->setInterval(msecs);

    if (wasActive)
        m_timer->start();
}

int StateChecker::interval() const
{
    return m_timer->interval();
}

void StateChecker::check()
{
    m_bus->performAction(m_actCheck.get());
}

void StateChecker::onQueryFinish(const Action::Id & /*id*/, const QString &result, bool /*ok*/,
                                 const QString & /*info*/)
{
    auto future = QtConcurrent::run([this, result]() {
        try {
            updateState(result);
        } catch (const std::exception &e) {
            WRN << "Exception in async task:" << e.what();
        }
    });

    auto *watcher = new QFutureWatcher<void>(this);
    QObject::connect(watcher, &QFutureWatcher<void>::finished, this, [future, watcher]() {
        if (future.isCanceled()) {
            WRN << "Async task was canceled!";
        }
        LOG << "Async task finished.";
        watcher->deleteLater();
    });
    watcher->setFuture(future);
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
        if (m_state.status() != state.status() || m_state.country() != state.country()
            || m_state.city() != state.city())
            emit statusChanged(state.status());

        m_state = state;
        emit stateChanged(m_state);
    }
}

void StateChecker::setStatus(NordVpnInfo::Status status)
{
    if (m_state.status() != status) {
        NordVpnInfo state;
        if (status != NordVpnInfo::Status::Unknown)
            state = m_state;

        state.setStatus(status);
        setState(state);
    }
}
