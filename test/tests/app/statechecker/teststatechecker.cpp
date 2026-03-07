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

#include "actions/actionstorage.h"
#include "app/statechecker.h"
#include "cli/clicaller.h"
#include "settings/appsettings.h"
#include "testutils.h"

#include <QElapsedTimer>
#include <QObject>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QTimer>

class TestStateChecker : public QObject
{
    Q_OBJECT
public:
    explicit TestStateChecker(QObject *parent = {});

private slots:
    void onStatusCheckPerformed(const NordVpnInfo::Status &status);

    void initTestCase();

    void test_active();
    void test_interval();
    void test_no_overlapping_polls();
    void test_error_resilience();
    void test_polling_mode_switch();
    void test_transition_polling();
    void test_tick_uptime();
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

void TestStateChecker::test_active()
{
    QSignalSpy spy(m_checker.get(), &StateChecker::statusChanged);

    QCOMPARE(m_checker->isActive(), false);
    m_checker->setActive(true); // triggers StateChecker::check which triggers StateChecker::statusChanged
                                // which may be wrongly caught by upfollowing tests
    QCOMPARE(m_checker->isActive(), true);
    m_checker->setActive(false);
    QCOMPARE(m_checker->isActive(), false);

    testutils::waitForSpyOrFail(spy);
}

void TestStateChecker::test_no_overlapping_polls()
{
    // m_pollInFlight is accessible because TestStateChecker is a friend.
    QCOMPARE(m_checker->m_pollInFlight, false);

    // Spy on Action::performed — fires unconditionally on every completed poll,
    // regardless of whether the state changed. This avoids a false timeout when
    // the CLI returns the same status that is already stored.
    const Action::Ptr &action = m_storage->action(Action::NordVPN::CheckStatus);
    QSignalSpy spy(action.get(), &Action::performed);

    // First call dispatches the CLI request and sets the flag synchronously.
    m_checker->check();
    QVERIFY(m_checker->m_pollInFlight);

    // Second call while the first is in flight must be a silent no-op.
    m_checker->check();
    QVERIFY(m_checker->m_pollInFlight);

    // Wait for the single poll to complete.
    testutils::waitForSpyOrFail(spy, CLICall::DefaultTimeoutMSecs);

    QCOMPARE(m_checker->m_pollInFlight, false);
    QCOMPARE(spy.count(), 1); // exactly one result despite two check() calls
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

void TestStateChecker::test_polling_mode_switch()
{
    // Previous tests may have left a transition in progress; reset to stable.
    m_checker->endTransition();

    // Default mode is Dynamic — timer runs at the stable dynamic interval.
    QCOMPARE(m_checker->m_pollingMode, StateChecker::PollingMode::Dynamic);
    QCOMPARE(m_checker->m_timer->interval(), StateChecker::DynamicIntervalStableMs);

    // Switch to Custom — timer must use the stored custom interval.
    m_checker->setPollingMode(StateChecker::PollingMode::Custom);
    QCOMPARE(m_checker->m_pollingMode, StateChecker::PollingMode::Custom);
    QCOMPARE(m_checker->m_timer->interval(), m_checker->m_customIntervalMs);

    // setInterval in Custom mode must update the timer immediately.
    static constexpr int newInterval = 7000;
    m_checker->setInterval(newInterval);
    QCOMPARE(m_checker->interval(), newInterval);
    QCOMPARE(m_checker->m_timer->interval(), newInterval);

    // setInterval in Dynamic mode must store the value but not touch the timer.
    m_checker->setPollingMode(StateChecker::PollingMode::Dynamic);
    static constexpr int anotherInterval = 9000;
    m_checker->setInterval(anotherInterval);
    QCOMPARE(m_checker->interval(), anotherInterval); // stored
    QVERIFY(m_checker->m_timer->interval() != anotherInterval); // timer unchanged

    // Restore defaults for subsequent tests.
    m_checker->setInterval(StateChecker::DefaultIntervalMs);
}

void TestStateChecker::test_transition_polling()
{
    // Ensure we start in Dynamic mode with no active transition.
    m_checker->setPollingMode(StateChecker::PollingMode::Dynamic);
    m_checker->endTransition();
    QVERIFY(!m_checker->m_transitionTimer->isActive());
    QCOMPARE(m_checker->m_timer->interval(), StateChecker::DynamicIntervalStableMs);

    // A status change must trigger fast (transitional) polling.
    m_checker->setStatus(NordVpnInfo::Status::Connecting);
    QVERIFY(m_checker->m_transitionTimer->isActive());
    QCOMPARE(m_checker->m_timer->interval(), StateChecker::DynamicIntervalTransitionalMs);

    // A further status change must reset the timeout and keep fast polling.
    m_checker->setStatus(NordVpnInfo::Status::Connected);
    QVERIFY(m_checker->m_transitionTimer->isActive());
    QCOMPARE(m_checker->m_timer->interval(), StateChecker::DynamicIntervalTransitionalMs);

    // Ending the transition explicitly must revert to stable polling.
    m_checker->endTransition();
    QVERIFY(!m_checker->m_transitionTimer->isActive());
    QCOMPARE(m_checker->m_timer->interval(), StateChecker::DynamicIntervalStableMs);

    // In Custom mode a status change must not affect the polling interval.
    m_checker->setPollingMode(StateChecker::PollingMode::Custom);
    const int savedCustom = m_checker->m_customIntervalMs;
    m_checker->setInterval(7000);
    m_checker->setStatus(NordVpnInfo::Status::Disconnected);
    QCOMPARE(m_checker->m_timer->interval(), 7000);
    QVERIFY(!m_checker->m_transitionTimer->isActive());

    // Restore.
    m_checker->setPollingMode(StateChecker::PollingMode::Dynamic);
    m_checker->setInterval(savedCustom);
}

void TestStateChecker::test_tick_uptime()
{
    // Build a Connected NordVpnInfo with known uptime "3 hours 24 minutes 5 seconds"
    // → parseUptime → "00:03:24:05"
    const QString connectedBase = QStringLiteral("Status: Connected\nCurrent server: fi88.nordvpn.com\n"
                                                 "Country: Finland\nCity: Helsinki\nYour new IP: 196.196.203.67\n"
                                                 "Current technology: OpenVPN\nCurrent protocol: UDP\n"
                                                 "Transfer: 0.97 MiB received, 452.22 KiB sent\nUptime: %1");

    NordVpnInfo info = NordVpnInfo::fromString(connectedBase.arg("3 hours 24 minutes 5 seconds"));
    const NordVpnInfo expected = NordVpnInfo::fromString(connectedBase.arg("3 hours 24 minutes 6 seconds"));

    info.tickUptime();
    QCOMPARE(info, expected);

    // Verify rollover: 59 seconds → 1 minute 0 seconds
    NordVpnInfo atRollover = NordVpnInfo::fromString(connectedBase.arg("59 seconds"));
    const NordVpnInfo afterRollover = NordVpnInfo::fromString(connectedBase.arg("1 minutes 0 seconds"));
    atRollover.tickUptime();
    QCOMPARE(atRollover, afterRollover);
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

    m_detectedStatus = sourceStatus;

    QSignalSpy spy(m_checker.get(), &StateChecker::statusChanged);

    m_checker->check();

    testutils::waitForSpyOrFail(spy, CLICall::DefaultTimeoutMSecs);

    QCOMPARE(m_checker->state().status(), targetStatus);
    QVERIFY(spy.count() >= 1);

    const QList<QVariant> &arguments = spy.takeLast();
    const auto &arg = arguments.at(0);
    QVERIFY(arg.metaType() == QMetaType::fromType<NordVpnInfo::Status>());
    QVERIFY(arg.value<NordVpnInfo::Status>() == targetStatus);
}

void TestStateChecker::test_error_resilience()
{
    const Action::Ptr &action = m_storage->action(Action::NordVPN::CheckStatus);
    const QString savedApp = action->app();

    testutils::ignoreWarning(QStringLiteral("Target binary file not exists:"));
    testutils::ignoreWarning(QStringLiteral("Failed to dispatch status check"));

    // Point the action at a non-existent binary to force CLI errors.
    action->setApp(QStringLiteral("/nonexistent/yangl-test-binary"));

    QSignalSpy errorSpy(m_checker.get(), &StateChecker::error);

    // Trigger errors one at a time; monitor must survive the first N-1 of them.
    for (int i = 0; i < StateChecker::MaxConsecutiveErrors - 1; ++i) {
        testutils::ignoreWarning(QStringLiteral("Target binary file not exists:"));
        testutils::ignoreWarning(QStringLiteral("Failed to dispatch status check"));

        m_checker->check();
        if (errorSpy.size() <= i) {
            testutils::waitForSpyOrFail(errorSpy, CLICall::DefaultTimeoutMSecs);
        }
        QCOMPARE(errorSpy.count(), i + 1);
        QCOMPARE(m_checker->m_consecutiveErrors, i + 1); // counter advancing
    }

    testutils::ignoreWarning(QStringLiteral("Stopping monitor after 3 consecutive errors"));

    // The Nth error must trip the threshold: counter resets and monitor stops.
    m_checker->check();
    if (errorSpy.size() < StateChecker::MaxConsecutiveErrors) {
        testutils::waitForSpyOrFail(errorSpy, CLICall::DefaultTimeoutMSecs);
    }

    QCOMPARE(errorSpy.count(), StateChecker::MaxConsecutiveErrors);
    QCOMPARE(m_checker->m_consecutiveErrors, 0); // reset after stop

    // Restore the good binary and verify a successful poll resets the counter.
    action->setApp(savedApp);
    action->setArgs({ "--status-disconnected" });

    QSignalSpy stateSpy(m_checker.get(), &StateChecker::stateChanged);
    m_checker->check();
    testutils::waitForSpyOrFail(stateSpy, CLICall::DefaultTimeoutMSecs);

    QCOMPARE(m_checker->m_consecutiveErrors, 0);
}

void TestStateChecker::test_check_status_change()
{
    QCOMPARE(m_checker->state().status(), NordVpnInfo::Status::Disconnected);

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
