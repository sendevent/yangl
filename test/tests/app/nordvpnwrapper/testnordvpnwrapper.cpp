/*
   Copyright (C) 2025-2026 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/testaction.h"
#include "app/nordvpnwrapper.h"
#include "settings/appsettings.h"
#include "testutils.h"

#include <QResource>
#include <QSignalSpy>
#include <QTest>
#include <memory>

// Expose the protected receivers() for verification
class TestableAction : public TestAction
{
public:
    using TestAction::TestAction;
    int invocationErrorReceiverCount() const { return receivers(SIGNAL(invocationError(QString))); }
};

class TestNordVpnWrapper : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_registerAction_noInstance();
    void test_registerAction_nullAction();
    void test_notice_dedupUntilCleared();
};

class NordVpnWrapperTestHook
{
public:
    static std::unique_ptr<NordVpnWrapper> create() { return std::unique_ptr<NordVpnWrapper>(new NordVpnWrapper); }
    static void onStateChanged(NordVpnWrapper &wrapper, const NordVpnInfo &state) { wrapper.onStateChanged(state); }
    static bool noticeActive(const NordVpnWrapper &wrapper) { return wrapper.m_nordVpnUpdateNoticeActive; }
    static bool noticeNotified(const NordVpnWrapper &wrapper) { return wrapper.m_nordVpnUpdateNoticeNotified; }

    static void clearQueue(NordVpnWrapper &wrapper) { wrapper.m_updateNotificationQueue.clear(); }
    static void setInFlight(NordVpnWrapper &wrapper, bool value) { wrapper.m_updateNotificationInFlight = value; }
    static int queueSize(const NordVpnWrapper &wrapper) { return wrapper.m_updateNotificationQueue.size(); }
    static bool inFlight(const NordVpnWrapper &wrapper) { return wrapper.m_updateNotificationInFlight; }

    static void enqueueNotification(NordVpnWrapper &wrapper, const std::function<void()> &cb)
    {
        wrapper.enqueueUpdateNotification(cb);
    }
    static void processNextNotification(NordVpnWrapper &wrapper) { wrapper.processNextUpdateNotification(); }
};

static void ignoreExpectedTrayWarnings()
{
    // In test binaries resources may be absent, which makes tray icon generation noisy.
    const bool hasTrayResource = QResource(QStringLiteral(":/icn/resources/tray/unknown.png")).isValid();
    if (!hasTrayResource) {
        for (int i = 0; i < 11; ++i) {
            testutils::ignoreWarning(QStringLiteral("Failed to deploy default icon"));
        }
        for (int i = 0; i < 5; ++i) {
            testutils::ignoreWarning(QStringLiteral("QPixmap::scaled: Pixmap is a null pixmap"));
            testutils::ignoreWarning(QStringLiteral("QPainter::begin: Paint device returned engine == 0, type: 2"));
        }
        for (int i = 0; i < 10; ++i) {
            testutils::ignoreWarning(
                    QStringLiteral("QPainter::setRenderHint: Painter must be active to set rendering hints"));
        }
        testutils::ignoreWarning(QStringLiteral("QSystemTrayIcon::setVisible: No Icon set"));
    }
}

void TestNordVpnWrapper::initTestCase()
{
    AppSettings::init();
}

void TestNordVpnWrapper::test_registerAction_noInstance()
{
    // In the test environment NordVpnWrapper::instance() is never called,
    // so s_instance is nullptr. registerAction must silently skip the
    // signal connection instead of triggering singleton construction.
    TestableAction action;

    QCOMPARE(action.invocationErrorReceiverCount(), 0);
    NordVpnWrapper::registerAction(&action);
    QCOMPARE(action.invocationErrorReceiverCount(), 0);
}

void TestNordVpnWrapper::test_registerAction_nullAction()
{
    // Must not crash when called with nullptr
    NordVpnWrapper::registerAction(nullptr);
}

void TestNordVpnWrapper::test_notice_dedupUntilCleared()
{
    ignoreExpectedTrayWarnings();

    auto wrapper = NordVpnWrapperTestHook::create();

    QSignalSpy spy(wrapper.get(), &NordVpnWrapper::nordVpnUpdateAvailable);
    const NordVpnInfo withNotice =
            NordVpnInfo::fromString("A new version of NordVPN is available!\nStatus: Connected\nServer: s");
    const NordVpnInfo withoutNotice = NordVpnInfo::fromString("Status: Connected\nServer: s");

    NordVpnWrapperTestHook::onStateChanged(*wrapper, withNotice);
    QCOMPARE(spy.count(), 1);
    QVERIFY(NordVpnWrapperTestHook::noticeActive(*wrapper));
    QVERIFY(NordVpnWrapperTestHook::noticeNotified(*wrapper));

    NordVpnWrapperTestHook::onStateChanged(*wrapper, withNotice);
    QCOMPARE(spy.count(), 1); // deduplicated while notice remains present

    NordVpnWrapperTestHook::onStateChanged(*wrapper, withoutNotice);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!NordVpnWrapperTestHook::noticeActive(*wrapper));
    QVERIFY(!NordVpnWrapperTestHook::noticeNotified(*wrapper));

    NordVpnWrapperTestHook::onStateChanged(*wrapper, withNotice);
    QCOMPARE(spy.count(), 2); // emitted again after notice disappears and reappears
}

QTEST_MAIN(TestNordVpnWrapper)
#include "testnordvpnwrapper.moc"
