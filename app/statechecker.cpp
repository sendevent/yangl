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

#include "ipcbus.h"
#include "ipccall.h"

#include <QDebug>
#include <QFuture>
#include <QThread>
#include <QTimer>
#include <QtConcurrent>

#define LOG qDebug() << Q_FUNC_INFO << QThread::currentThreadId()
#define WRN qWarning() << Q_FUNC_INFO << QThread::currentThreadId()

static const int DefaultIntervalMs = 1000;
StateChecker::StateChecker(IPCBus *bus, QObject *parent)
    : QObject(parent)
    , m_bus(bus)
    , m_query(nullptr)
    , m_timer(new QTimer(this))
    , m_state(State::Unknown)
{
    qRegisterMetaType<StateChecker::State>("StateChecker::State");

    setInterval(DefaultIntervalMs);
    connect(m_timer, &QTimer::timeout, this, &StateChecker::onTimeout);
    setState(State::Unknown);
}

void StateChecker::setActive(bool active)
{
    LOG;

    if (m_timer->isActive() != active) {

        if (active) {
            check();
            m_timer->start();
        } else {
            m_timer->stop();
            setState(State::Unknown);
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

int StateChecker::inteval() const
{
    return m_timer->interval();
}

void StateChecker::check()
{
    IPCCall::Ptr statusCheck(new IPCCall(m_bus->applicationPath(), { QStringLiteral("status") }, 30000));
    connect(statusCheck.get(), &IPCCall::ready, this, &StateChecker::onQueryFinish);

    m_calls.enqueue(statusCheck);

    nextQuery();
}

void StateChecker::onQueryFinish(const QString &result)
{
    LOG << result;

    QFuture<void> future = QtConcurrent::run(this, &StateChecker::updateState, result);
    //    future.waitForFinished();

    m_query.clear();
    nextQuery();
}

void StateChecker::nextQuery()
{
    if (m_query)
        return;

    if (m_calls.isEmpty())
        return;

    m_query = m_calls.dequeue();

    m_bus->runQuery(m_query);
}

void StateChecker::onTimeout()
{
    check();
}

void StateChecker::updateState(const QString &from)
{
    LOG << from;

    if (from.isEmpty())
        return;

    const QStringList &pairs = from.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : pairs) {
        const QStringList &pair = line.split(':', Qt::SkipEmptyParts);
        if (pair.size() != 2) {
            WRN << "Unexpected format:" << line;
            continue;
        }

        const QString &name = pair.first().simplified();
        const QString &value = pair.last().simplified();

        if (name == QStringLiteral("- - Status")) {
            const QMetaEnum me = QMetaEnum::fromType<StateChecker::State>();
            bool found(false);
            const int enumVal = me.keyToValue(qPrintable(value), &found);
            setState(found ? static_cast<StateChecker::State>(enumVal) : StateChecker::Unknown);
            return;
        }
    }
}

StateChecker::State StateChecker::state() const
{
    return m_state;
}

void StateChecker::setState(StateChecker::State state)
{
    if (this->state() != state) {
        m_state = state;
        emit stateChanged(m_state);
    }
}
