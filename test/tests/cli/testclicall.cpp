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

#include "actions/testaction.h"
#include "cli/clicall.h"

#include <QSignalSpy>
#include <QTest>

class TestCLICall : public QObject
{
    Q_OBJECT
private slots:
    void test_call();
};

void TestCLICall::test_call()
{
    const Action::Ptr action(new TestAction());
    action->setApp("/usr/bin/ls");
    action->setArgs({ "-la" });

    CLICall *call = action->createRequest();
    QSignalSpy spy(call, &CLICall::ready);

    QString receivedResult;
    connect(call, &CLICall::ready, this, [&receivedResult](const QString &result) { receivedResult = result; });
    call->run();

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> &arguments = spy.takeFirst();
    const QString &filesList = arguments.at(0).toString();
    qDebug() << filesList;
    QVERIFY(arguments.at(0).typeId() == QVariant::String);
    QCOMPARE(filesList, receivedResult);
    QVERIFY(filesList.contains("Test_CLICall"));

    QCOMPARE(call->result(), filesList);
    QCOMPARE(call->exitCode(), 0);
    QCOMPARE(call->exitStatus(), QProcess::ExitStatus::NormalExit);
    QCOMPARE(call->result(), filesList);
    QCOMPARE(call->errors(), QString());
}

QTEST_MAIN(TestCLICall)
#include "testclicall.moc"
