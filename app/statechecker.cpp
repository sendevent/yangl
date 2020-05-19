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
#include "clibus.h"
#include "clicall.h"

#include <QDateTime>
#include <QDebug>
#include <QFuture>
#include <QThread>
#include <QTimer>
#include <QtConcurrent>

#define LOG qDebug() << Q_FUNC_INFO << QThread::currentThreadId()
#define WRN qWarning() << Q_FUNC_INFO << QThread::currentThreadId()

static const int DefaultIntervalMs = 1000;

void StateChecker::Info::clear()
{
    m_status = Status::Unknown;
    m_server.clear();
    m_country.clear();
    m_city.clear();
    m_ip.clear();
    m_technology.clear();
    m_protocol.clear();
    m_traffic.clear();
    m_uptime.clear();
}

bool StateChecker::Info::operator==(const Info &other) const
{
    return m_status == other.m_status && m_server == other.m_server && m_country == other.m_country
            && m_city == other.m_city && m_ip == other.m_ip && m_technology == other.m_technology
            && m_protocol == other.m_protocol && m_traffic == other.m_traffic && m_uptime == other.m_uptime;
}

bool StateChecker::Info::operator!=(const Info &other) const
{
    return !this->operator==(other);
}

/*static*/ StateChecker::Info StateChecker::Info::fromString(const QString &text)
{
    Info updatedState;
    if (text.isEmpty())
        return updatedState;

    const QStringList &pairs = text.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : pairs) {
        const QStringList &pair = line.split(':', Qt::SkipEmptyParts);
        if (pair.size() != 2) {
            WRN << "Unexpected format:" << line;
            continue;
        }

        const QString &name = pair.first().simplified();
        const QString &value = pair.last().simplified();

        if (name == QStringLiteral("- - Status"))
            updatedState.m_status = textToStatus(value);
        else if (name == QStringLiteral("Current server"))
            updatedState.m_server = value;
        else if (name == QStringLiteral("Country"))
            updatedState.m_country = value;
        else if (name == QStringLiteral("City"))
            updatedState.m_city = value;
        else if (name == QStringLiteral("Your new IP"))
            updatedState.m_ip = value;
        else if (name == QStringLiteral("Current technology"))
            updatedState.m_technology = value;
        else if (name == QStringLiteral("Current protocol"))
            updatedState.m_protocol = value;
        else if (name == QStringLiteral("Transfer")) {
            updatedState.m_traffic = value;
            updatedState.m_traffic.replace(QString("received"), QString("↓"));
            updatedState.m_traffic.replace(QString("sent"), QString("↑"));
        } else if (name == QStringLiteral("Uptime"))
            updatedState.m_uptime = parseUptime(value.simplified());
    }

    return updatedState;
}

/*static*/ StateChecker::Status StateChecker::Info::textToStatus(const QString &from)
{
    if (from.isEmpty())
        return StateChecker::Status::Unknown;

    const QMetaEnum me = QMetaEnum::fromType<StateChecker::Status>();
    bool found(false);
    const int enumVal = me.keyToValue(qPrintable(from), &found);
    return (found ? static_cast<StateChecker::Status>(enumVal) : StateChecker::Status::Unknown);
}

/*static*/ QString StateChecker::Info::statusToText(StateChecker::Status from)
{
    const QMetaEnum me = QMetaEnum::fromType<StateChecker::Status>();
    return me.valueToKey(from);
}

/*static*/ QString StateChecker::Info::parseUptime(const QString &from)
{
    QString result;

    if (from.isEmpty())
        return result;

    auto add = [&result](int value, int width = 2) {
        if (!result.isEmpty())
            result.append(":");
        result += QString("%1").arg(value, width, 10, QChar('0'));
    };

    const QStringList &parts = from.split(" ", Qt::SkipEmptyParts);
    for (int i = 0; i <= parts.size() - 2; ++i) {
        bool converted(false);
        const int value = parts.at(i).toInt(&converted);
        if (!converted)
            continue;

        const QString &units = parts.at(++i);

        if (units.startsWith("day"))
            add(value, 3);
        else
            add(value, 2);
    }

    if (result.count(":") <= 2)
        result.prepend("00:");

    return result;
}

QString StateChecker::Info::toString() const
{
    QString text;
    text.append(tr("<b>%1</b>").arg(statusToText(m_status)));

    if (m_status != StateChecker::Status::Connected && m_status != StateChecker::Status::Connecting)
        return text;

    auto add = [&text](const QString &str, const QString &delim = "<br>") {
        if (!text.isEmpty())
            text.append(delim);
        text.append(str);
        return text;
    };

    if (!m_uptime.isEmpty())
        text = add(m_uptime, " ");
    text = add(m_server);
    text = add(m_city, " — ");
    text = add(m_country, ", ");
    text = add(m_ip);
    text = add(m_technology);
    text = add(m_protocol, ", ");
    text = add(m_traffic);

    return text;
}

StateChecker::StateChecker(CLIBus *bus, ActionStorage *actions, QObject *parent)
    : QObject(parent)
    , m_bus(bus)
    , m_actions(actions)
    , m_currAction(nullptr)
    , m_timer(new QTimer(this))
    , m_state()
{
    qRegisterMetaType<StateChecker::Info>("StateChecker::Info");
    qRegisterMetaType<StateChecker::Status>("StateChecker::Status");

    setInterval(DefaultIntervalMs);
    connect(m_timer, &QTimer::timeout, this, &StateChecker::onTimeout);
    setStatus(StateChecker::Status::Unknown);
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
            setStatus(StateChecker::Status::Unknown);
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
    setState(Info::fromString(from));
}

StateChecker::Info StateChecker::state() const
{
    return m_state;
}

void StateChecker::setState(const StateChecker::Info &state)
{
    if (this->state() != state) {
        if (m_state.m_status != state.m_status)
            emit statusChanged(state.m_status);

        m_state = state;
        emit stateChanged(m_state);
    }
}

void StateChecker::setStatus(StateChecker::Status status)
{
    if (m_state.m_status != status) {
        Info state;
        if (status != Unknown)
            state = m_state;

        state.m_status = status;
        setState(state);
    }
}
