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

#include "tst_clicaller.h"

#include "clicaller.h"
#include "tst_action.h"

#include <QElapsedTimer>
#include <QSignalSpy>
#include <QtTest>
#include <memory>

void tst_CLICaller::test_performAction()
{
    Action::Ptr action(new tst_Action(Action::Scope::User, KnownAction::Unknown));
    action->setApp("/usr/bin/ls");
    action->setArgs({ "-la" });

    bool actionPerformed(false);
    connect(action.get(), &Action::performed, this,
            [&actionPerformed](const Action::Id & /*id*/, const QString & /*result*/, bool /*ok*/,
                               const QString & /*info*/) { actionPerformed = true; });

    QSignalSpy spy(action.get(), &Action::performed);

    std::unique_ptr<CLICaller> caller(new CLICaller);
    caller->performAction(action.get());

    QElapsedTimer timer;
    timer.start();
    while (!actionPerformed && timer.elapsed() < CLICall::DefaultTimeoutMSecs)
        QTest::qWait(10);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).type() == QVariant::Uuid);
    QVERIFY(arguments.at(1).type() == QVariant::String);
    QVERIFY(arguments.at(2).type() == QVariant::Bool);
    QVERIFY(arguments.at(3).type() == QVariant::String);
    QVERIFY(arguments.at(2).toBool() == true);
}
