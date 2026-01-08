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

#include <QTest>

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
    void test_registerAction_noInstance();
    void test_registerAction_nullAction();
};

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

QTEST_MAIN(TestNordVpnWrapper)
#include "testnordvpnwrapper.moc"
