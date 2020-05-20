/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "tst_clicall.h"

#include "clicall.h"
#include "tst_action.h"

#include <QSignalSpy>
#include <QtTest>

tst_CLICall::tst_CLICall(QObject *parent)
    : QObject(parent)
{
}

void tst_CLICall::testCall()
{
    const Action::Ptr action(new tst_Action());
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
    QVERIFY(arguments.at(0).type() == QVariant::String);
    QCOMPARE(filesList, receivedResult);
    QVERIFY(filesList.contains("tst_clicall.o"));

    QCOMPARE(call->result(), filesList);
    QCOMPARE(call->exitCode(), 0);
    QCOMPARE(call->exitStatus(), QProcess::ExitStatus::NormalExit);
    QCOMPARE(call->result(), filesList);
    QCOMPARE(call->errors(), QString());
}
