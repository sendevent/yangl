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

#include "testaction.h"

#include "actions/action.h"
#include "actions/actionstorage.h"
#include "cli/clicall.h"

#include <QSignalSpy>
#include <QTest>

/*static*/ int TestAction::MetaIdMenuPlace = -1;

TestAction::TestAction(Action::Flow scope, NordVPN action, ActionStorage *parent, const Action::Id &id)
    : Action(scope, static_cast<int>(action), parent, id)
{
    if (-1 == TestAction::MetaIdMenuPlace)
        TestAction::MetaIdMenuPlace = qRegisterMetaType<Action::MenuPlace>();
}

void TestAction::checkAction(const Action::Ptr &action, int expectedType, Action::Flow expectedScope,
                             const Action::Id &expectedId) const
{
    QCOMPARE(action->type(), expectedType);
    QCOMPARE(action->scope(), expectedScope);

    if (expectedId.isNull())
        QVERIFY(!action->id().isNull());
    else
        QCOMPARE(action->id(), expectedId);

    QVERIFY(action->title().isEmpty());
    QVERIFY(action->app().isEmpty());
    QVERIFY(action->args().isEmpty());
    QCOMPARE(action->timeout(), CLICall::DefaultTimeoutMSecs);
    QVERIFY(!action->forcedShow());
    QCOMPARE(action->anchor(), Action::MenuPlace::NoMenu);
}

void TestAction::testCreate_Builtin()
{
    static const Action::Flow scope = Action::Flow::NordVPN;
    for (auto actionType : Action::nvpnActions()) {
        static const Action::Id &id = Action::Id::createUuid();

        const Action::Ptr action(new TestAction(scope, actionType, {}, id));
        checkAction(action, static_cast<int>(actionType), scope, id);

        const Action::Ptr actionNoId(new TestAction(scope, actionType));
        checkAction(actionNoId, static_cast<int>(actionType), scope);
    }
}

void TestAction::testCreate_Custom()
{
    static const Action::Flow scope = Action::Flow::Custom;
    static const Action::Id &id = Action::Id::createUuid();
    static const NordVPN actionType = NordVPN::Unknown;

    const Action::Ptr action(new TestAction(scope, actionType, {}, id));
    checkAction(action, static_cast<int>(actionType), scope, id);

    const Action::Ptr actionNoId(new TestAction(scope, actionType));
    checkAction(actionNoId, static_cast<int>(actionType), scope);
}

void TestAction::testSetTitle()
{
    static const QString testValue("test");
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::titleChanged);

    action->setTitle(testValue);

    QCOMPARE(action->title(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::String);
    QVERIFY(arguments.at(0) == testValue);
}

void TestAction::testSetApp()
{
    static const QString testValue("/no/such/app");
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::appChanged);

    action->setApp(testValue);

    QCOMPARE(action->app(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::String);
    QVERIFY(arguments.at(0) == testValue);
}

void TestAction::testSetArgs()
{
    static const QStringList testValue { "-a", "\"b c d\"", "e" };
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::argsChanged);

    action->setArgs(testValue);

    QCOMPARE(action->args(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::StringList);
    QVERIFY(arguments.at(0) == testValue);
}

void TestAction::testSetTimeout()
{
    static const int testValue(1);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::timeoutChanged);

    action->setTimeout(testValue);

    QCOMPARE(action->timeout(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::Int);
    QVERIFY(arguments.at(0) == testValue);
}

void TestAction::testSetForcedShow()
{
    static const bool testValue(true);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::forcedShowChanged);

    action->setForcedShow(testValue);

    QCOMPARE(action->forcedShow(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).typeId() == QVariant::Bool);
    QVERIFY(arguments.at(0) == testValue);
}

void TestAction::testSetAnchor()
{
    static const Action::MenuPlace testValue(Action::MenuPlace::Own);
    const Action::Ptr action(new TestAction(Action::Flow::Custom, NordVPN::Unknown));
    QSignalSpy spy(action.get(), &Action::anchorChanged);

    action->setAnchor(testValue);

    QCOMPARE(action->anchor(), testValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<Action::MenuPlace>(), testValue);
    QCOMPARE(arguments.at(0).toInt(), static_cast<int>(testValue));
}
