/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/actionstorage.h"
#include "app/statechecker.h"
#include "cli/clicaller.h"
#include "settings/appsettings.h"

#include <QObject>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

class TestStateChecker : public QObject
{
    Q_OBJECT
public:
    explicit TestStateChecker(QObject *parent = {});

private slots:
    void onStatusCheckPerformed(const NordVpnInfo::Status &status);

    void initTestCase();
    void init();

    void test_active();
    void test_interval();
    void test_check_status_change();

private:
    const std::unique_ptr<CLICaller> m_caller;
    const std::unique_ptr<ActionStorage> m_storage;
    StateChecker::Ptr m_checker;
    NordVpnInfo::Status m_detectedStatus;

    void test_check(NordVpnInfo::Status targetStatus);
};

TestStateChecker::TestStateChecker(QObject *parent)
    : QObject(parent)
    , m_caller(new CLICaller)
    , m_storage(new ActionStorage)
{
}

void TestStateChecker::initTestCase()
{
    AppSettings::init();
    m_storage->load();
    const Action::Ptr &action = m_storage->action(Action::NordVPN::CheckStatus);
    const QFileInfo fakeStatusApp(
            QString(qApp->applicationFilePath()).replace(qAppName(), "../../../../../tests/test_fake_status"));
    action->setApp(fakeStatusApp.absoluteFilePath());
    action->setArgs({ "-d" });
    action->setForcedShow(false);

    m_checker = StateChecker::Ptr(new StateChecker(m_caller.get(), StateChecker::DefaultIntervalMs));
    m_checker->setCheckAction(action);

    connect(m_checker.get(), &StateChecker::statusChanged, this, &TestStateChecker::onStatusCheckPerformed);
}

void TestStateChecker::init() { }

void TestStateChecker::test_active()
{
    QCOMPARE(m_checker->isActive(), false);
    m_checker->setActive(true);
    QCOMPARE(m_checker->isActive(), true);
    m_checker->setActive(false);
    QCOMPARE(m_checker->isActive(), false);
}

void TestStateChecker::test_interval()
{
    QCOMPARE(m_checker->interval(), StateChecker::DefaultIntervalMs);
    static int constexpr customInterval = 3600000;
    m_checker->setInterval(customInterval);
    QCOMPARE(m_checker->interval(), customInterval);
    m_checker->setInterval(StateChecker::DefaultIntervalMs);
    QCOMPARE(m_checker->interval(), StateChecker::DefaultIntervalMs);
}

void TestStateChecker::onStatusCheckPerformed(const NordVpnInfo::Status &status)
{
    m_detectedStatus = status;
}

void TestStateChecker::test_check(NordVpnInfo::Status targetStatus)
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

    qDebug() << action->app() << action->args();

    m_detectedStatus = sourceStatus;

    QSignalSpy spy(m_checker.get(), &StateChecker::statusChanged);

    m_checker->check();

    QElapsedTimer timer;
    timer.start();
    while (m_detectedStatus == sourceStatus && timer.elapsed() < 20000)
        QTest::qWait(50);

    qDebug() << m_checker->state().toString();
    QCOMPARE(m_checker->state().status(), targetStatus);

    const QList<QVariant> &arguments = spy.takeLast();
    const auto &arg = arguments.at(0);
    qDebug() << arg << arg.typeId() << arg.metaType();
    QVERIFY(arg.metaType() == QMetaType::fromType<NordVpnInfo::Status>());
    QVERIFY(arg.value<NordVpnInfo::Status>() == targetStatus);
}

void TestStateChecker::test_check_status_change()
{
    QCOMPARE(m_checker->state().status(), NordVpnInfo::Status::Unknown);

    {
        QSignalSpy spyState(m_checker.get(), &StateChecker::stateChanged);
        QSignalSpy spyStatus(m_checker.get(), &StateChecker::statusChanged);

        m_checker->setStatus(NordVpnInfo::Status::Connecting);
        QCOMPARE(m_checker->state().status(), NordVpnInfo::Status::Connecting);

        {
            QCOMPARE(spyState.count(), 1);
            const QList<QVariant> &arguments = spyState.takeFirst();
            const auto &arg = arguments.at(0);
            QCOMPARE(arg.metaType(), QMetaType::fromType<NordVpnInfo>());
            QCOMPARE(arg.value<NordVpnInfo>().status(), NordVpnInfo::Status::Connecting);
        }
        {
            QCOMPARE(spyStatus.count(), 1);
            const QList<QVariant> &arguments = spyStatus.takeFirst();
            const auto &arg = arguments.at(0);
            QCOMPARE(arg.metaType(), QMetaType::fromType<NordVpnInfo::Status>());
            QCOMPARE(arg.value<NordVpnInfo::Status>(), NordVpnInfo::Status::Connecting);
        }
    }

    m_checker->setStatus(NordVpnInfo::Status::Disconnected);
    test_check(NordVpnInfo::Status::Connecting);

    m_checker->setStatus(NordVpnInfo::Status::Connecting);
    test_check(NordVpnInfo::Status::Connected);

    m_checker->setStatus(NordVpnInfo::Status::Connected);
    test_check(NordVpnInfo::Status::Disconnected);
}

QTEST_MAIN(TestStateChecker)
#include "teststatechecker.moc"
