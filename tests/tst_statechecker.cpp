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

#include "tst_statechecker.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest>

tst_StateChecker::tst_StateChecker(QObject *parent)
    : QObject(parent)
    , m_caller(new CLICaller)
    , m_storage(new ActionStorage)
{
    QStandardPaths::setTestModeEnabled(true);

    m_storage->load();
    const Action::Ptr &action = m_storage->action(Action::NordVPN::CheckStatus);
    QString statusSink(qApp->applicationFilePath());
    statusSink = statusSink.replace(qAppName(), "test_fake_status");
    action->setApp(statusSink);
    action->setArgs({ "" });
    action->setForcedShow(false);
}

void tst_StateChecker::init()
{
    m_checker = StateChecker::Ptr(new StateChecker(m_caller.get(), StateChecker::DefaultIntervalMs));
    m_checker->setCheckAction(m_storage->action(Action::NordVPN::CheckStatus));
}

void tst_StateChecker::test_active()
{
    QCOMPARE(m_checker->isActive(), false);
    m_checker->setActive(true);
    QCOMPARE(m_checker->isActive(), true);
    m_checker->setActive(false);
    QCOMPARE(m_checker->isActive(), false);
}

void tst_StateChecker::test_interval()
{
    QCOMPARE(m_checker->interval(), StateChecker::DefaultIntervalMs);
    static int constexpr customInterval = 3600000;
    m_checker->setInterval(customInterval);
    QCOMPARE(m_checker->interval(), customInterval);
    m_checker->setInterval(StateChecker::DefaultIntervalMs);
    QCOMPARE(m_checker->interval(), StateChecker::DefaultIntervalMs);
}

void tst_StateChecker::test_check(NordVpnInfo::Status targetStatus)
{
    const NordVpnInfo::Status sourceStatus = m_checker->state().status();
    const Action::Ptr &action = m_storage->action(Action::NordVPN::CheckStatus);

    switch (targetStatus) {
    case NordVpnInfo::Status::Connecting:
        action->setArgs({ "--status-connecting" });
        break;
    case NordVpnInfo::Status::Connected:
        action->setArgs({ "--status-connected" });
        break;
    case NordVpnInfo::Status::Disconnected:
        action->setArgs({ "--status-disconnected" });
        break;
    default:
        return;
    }

    NordVpnInfo::Status checkPerformed(sourceStatus);
    connect(
            &*m_checker, &StateChecker::statusChanged, this,
            [&checkPerformed](const NordVpnInfo::Status &status) { checkPerformed = status; }, Qt::UniqueConnection);

    QSignalSpy spy(&*m_checker, &StateChecker::statusChanged);

    m_checker->check();

    QElapsedTimer timer;
    timer.start();
    while (checkPerformed == sourceStatus && timer.elapsed() < 20000)
        QTest::qWait(50);

    QCOMPARE(m_checker->state().status(), targetStatus);

    const QList<QVariant> &arguments = spy.takeLast();
    QVERIFY(arguments.at(0).type() == QVariant::UserType);
    QVERIFY(arguments.at(0).value<NordVpnInfo::Status>() == targetStatus);
}

void tst_StateChecker::test_check_status_change()
{
    QCOMPARE(m_checker->state().status(), NordVpnInfo::Status::Unknown);

    {
        QSignalSpy spyState(&*m_checker, &StateChecker::stateChanged);
        QSignalSpy spyStatus(&*m_checker, &StateChecker::statusChanged);

        m_checker->setStatus(NordVpnInfo::Status::Connecting);
        QCOMPARE(m_checker->state().status(), NordVpnInfo::Status::Connecting);

        {
            QCOMPARE(spyState.count(), 1);
            const QList<QVariant> &arguments = spyState.takeFirst();
            QVERIFY(arguments.at(0).type() == QVariant::UserType);
            QVERIFY(arguments.at(0).value<NordVpnInfo>().status() == NordVpnInfo::Status::Connecting);
        }
        {
            QCOMPARE(spyStatus.count(), 1);
            const QList<QVariant> &arguments = spyStatus.takeFirst();
            QVERIFY(arguments.at(0).type() == QVariant::UserType);
            QVERIFY(arguments.at(0).value<NordVpnInfo::Status>() == NordVpnInfo::Status::Connecting);
        }
    }

    m_checker->setStatus(NordVpnInfo::Status::Disconnected);
    test_check(NordVpnInfo::Status::Connecting);

    m_checker->setStatus(NordVpnInfo::Status::Connecting);
    test_check(NordVpnInfo::Status::Connected);

    m_checker->setStatus(NordVpnInfo::Status::Connected);
    test_check(NordVpnInfo::Status::Disconnected);
}
