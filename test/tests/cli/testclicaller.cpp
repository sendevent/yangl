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

#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTest>
#include <memory>

class TestCLICaller : public QObject
{
    Q_OBJECT

private slots:
    void test_performAction();
};

void TestCLICaller::test_performAction()
{
    Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown));
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
    QVERIFY(arguments.at(0).typeId() == QVariant::Uuid);
    QVERIFY(arguments.at(1).typeId() == QVariant::String);
    QVERIFY(arguments.at(2).typeId() == QVariant::Bool);
    QVERIFY(arguments.at(3).typeId() == QVariant::String);
    QVERIFY(arguments.at(2).toBool() == true);
}

QTEST_MAIN(TestCLICaller)
#include "testclicaller.moc"
