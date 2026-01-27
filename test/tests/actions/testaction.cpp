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
    if (-1 == TestAction::MetaIdMenuPlace) {
        TestAction::MetaIdMenuPlace = qRegisterMetaType<Action::MenuPlace>();
    }
}

void TestAction::checkAction(const Action::Ptr &action, int expectedType, Action::Flow expectedScope,
                             const Action::Id &expectedId) const
{
    QCOMPARE(action->type(), expectedType);
    QCOMPARE(action->scope(), expectedScope);

    if (expectedId.isNull()) {
        QVERIFY(!action->id().isNull());
    } else {
        QCOMPARE(action->id(), expectedId);
    }

    QVERIFY(action->title().isEmpty());
    QVERIFY(action->app().isEmpty());
    QVERIFY(action->args().isEmpty());
    QCOMPARE(action->timeout(), CLICall::DefaultTimeoutMSecs);
    QVERIFY(!action->forcedShow());
    QCOMPARE(action->anchor(), Action::MenuPlace::NoMenu);
}
