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

#include "actions/testaction.h"
#include "cli/clicaller.h"

#include <QSignalSpy>
#include <QTest>
#include <memory>

class TestCLICaller : public QObject
{
    Q_OBJECT

private slots:
    void test_performAction();
    void test_runCall_null();
    void test_performAction_failure();
};

void TestCLICaller::test_performAction()
{
    Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    action->setApp("/usr/bin/ls");
    action->setArgs({ "-la" });

    QSignalSpy spy(action.get(), &Action::performed);

    CLICall *call = action->createRequest();
    QVERIFY(call != nullptr);

    std::unique_ptr<CLICaller> caller(new CLICaller);
    QVERIFY(caller->runCall(call));

    if (spy.isEmpty()) {
        QVERIFY(spy.wait());
    }

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::Uuid);
    QVERIFY(arguments.at(1).typeId() == QVariant::String);
    QVERIFY(!arguments.at(1).toString().isEmpty());
    QVERIFY(arguments.at(2).typeId() == QVariant::Bool);
    QCOMPARE(arguments.at(2).toBool(), true);

    const auto runInfo = arguments.at(3).value<Action::RunInfo>();
    QVERIFY(runInfo.exitCode.isEmpty()); // exitCode is empty string on success (0)
    QVERIFY(!runInfo.result.isEmpty());
    QVERIFY(runInfo.errors.isEmpty());
    QVERIFY(!runInfo.timeStamp.isEmpty());
}

void TestCLICaller::test_runCall_null()
{
    CLICaller caller;
    QCOMPARE(caller.runCall(nullptr), false);
}

void TestCLICaller::test_performAction_failure()
{
    Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    action->setApp("/usr/bin/false");

    QSignalSpy spy(action.get(), &Action::performed);

    CLICall *call = action->createRequest();
    QVERIFY(call != nullptr);

    CLICaller caller;
    QVERIFY(caller.runCall(call));

    if (spy.isEmpty()) {
        QVERIFY(spy.wait());
    }

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QCOMPARE(arguments.at(2).toBool(), false);

    const auto runInfo = arguments.at(3).value<Action::RunInfo>();
    QVERIFY(!runInfo.exitCode.isEmpty());
    QVERIFY(!runInfo.timeStamp.isEmpty());
}

QTEST_MAIN(TestCLICaller)
#include "testclicaller.moc"
