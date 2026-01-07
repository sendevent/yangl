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

#include "testactiontests.h"

#include "actions/action.h"
#include "actions/actionstorage.h"
#include "cli/clicall.h"
#include "testaction.h"

#include <QSignalSpy>
#include <QTest>

void TestActionTests::testCreate_Builtin()
{
    const Action::Flow scope = Action::Flow::NordVPN;
    for (auto actionType : Action::nvpnActions()) {
        const Action::Id id = Action::Id::createUuid();

        const Action::Ptr action(new TestAction(scope, actionType, {}, id));
        TestAction().checkAction(action, static_cast<int>(actionType), scope, id);

        const Action::Ptr actionNoId(new TestAction(scope, actionType));
        TestAction().checkAction(actionNoId, static_cast<int>(actionType), scope);
    }
}

void TestActionTests::testCreate_Custom()
{
    const Action::Flow scope = Action::Flow::Custom;
    const Action::Id id = Action::Id::createUuid();
    const Action::NordVPN actionType = Action::NordVPN::Unknown;

    const Action::Ptr action(new TestAction(scope, actionType, {}, id));
    TestAction().checkAction(action, static_cast<int>(actionType), scope, id);

    const Action::Ptr actionNoId(new TestAction(scope, actionType));
    TestAction().checkAction(actionNoId, static_cast<int>(actionType), scope);
}

void TestActionTests::testSetTitle()
{
    const QString testValue("test");
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::titleChanged);

    action->setTitle(testValue);

    QCOMPARE(action->title(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::String);
    QVERIFY(arguments.at(0) == testValue);
}

void TestActionTests::testSetApp()
{
    const QString testValue("/no/such/app");
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::appChanged);

    action->setApp(testValue);

    QCOMPARE(action->app(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::String);
    QVERIFY(arguments.at(0) == testValue);
}

void TestActionTests::testSetArgs()
{
    const QStringList testValue { "-a", "\"b c d\"", "e" };
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::argsChanged);

    action->setArgs(testValue);

    QCOMPARE(action->args(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::StringList);
    QVERIFY(arguments.at(0) == testValue);
}

void TestActionTests::testSetTimeout()
{
    const int testValue(1);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::timeoutChanged);

    action->setTimeout(testValue);

    QCOMPARE(action->timeout(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::Int);
    QVERIFY(arguments.at(0) == testValue);
}

void TestActionTests::testSetForcedShow()
{
    const bool testValue(true);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::forcedShowChanged);

    action->setForcedShow(testValue);

    QCOMPARE(action->forcedShow(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::Bool);
    QVERIFY(arguments.at(0) == testValue);
}

void TestActionTests::testSetAnchor()
{
    const Action::MenuPlace testValue(Action::MenuPlace::Own);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::anchorChanged);

    action->setAnchor(testValue);

    QCOMPARE(action->anchor(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<Action::MenuPlace>(), testValue);
    QCOMPARE(arguments.at(0).toInt(), static_cast<int>(testValue));
}
