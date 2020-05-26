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

#include "statechecker.h"

#include "actionstorage.h"
#include "clicall.h"
#include "clicaller.h"
#include "common.h"

#include <QTimer>
#include <QtConcurrent>

/*static*/ const int StateChecker::DefaultIntervalMs = yangl::OneSecondMs;

StateChecker::StateChecker(CLICaller *bus, ActionStorage *actions, int intervalMs, QObject *parent)
    : QObject(parent)
    , m_bus(bus)
    , m_actions(actions)
    , m_currAction(nullptr)
    , m_timer(new QTimer(this))
    , m_state()
{
    setInterval(intervalMs);
    connect(m_timer, &QTimer::timeout, this, &StateChecker::onTimeout);
    setStatus(NordVpnInfo::Status::Unknown);
}

StateChecker::~StateChecker()
{
    m_calls.clear();
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
    }
}

bool StateChecker::isActive() const
{
    return m_timer->isActive();
}

void StateChecker::setInterval(int msecs)
{
    m_timer->setInterval(msecs);
    if (isActive()) {
        m_timer->stop();
        m_timer->start();
    }
}

int StateChecker::interval() const
{
    return m_timer->interval();
}

void StateChecker::check()
{
    if (auto statusAct = m_actions->action(KnownAction::CheckStatus)) {
        connect(statusAct.get(), &Action::performed, this, &StateChecker::onQueryFinish, Qt::UniqueConnection);
        m_calls.enqueue(statusAct);
        nextQuery();
    }
}

void StateChecker::onQueryFinish(const QString &result, bool ok)
{
    if (ok)
        QtConcurrent::run(this, &StateChecker::updateState, result);

    m_currAction.clear();
    nextQuery();
}

void StateChecker::nextQuery()
{
    if (m_currAction)
        return;

    if (m_calls.isEmpty())
        return;

    m_currAction = m_calls.dequeue();

    m_bus->performAction(m_currAction.get());
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
