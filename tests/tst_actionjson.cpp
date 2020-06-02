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

#include "tst_actionjson.h"

#include "actionjson.h"
#include "actionstorage.h"

#include <QBuffer>
#include <QtTest>

/*static*/ const Action::Id tst_ActionJson::TestId = Action::Id("{057c11c0-de87-48c4-a98e-93471b4290ca}");

/*static*/ const QByteArray tst_ActionJson::TestJson =
        "{\n    \"custom\": {\n        \"{057c11c0-de87-48c4-a98e-93471b4290ca}\": {\n            \"anchor\": 0,\n     "
        "       \"app\": \"/usr/bin/ls\",\n            \"args\": [\n                \"-la\"\n            ],\n          "
        "  \"forcedDisplay\": false,\n            \"id\": \"{057c11c0-de87-48c4-a98e-93471b4290ca}\",\n            "
        "\"scope\": 1,\n            \"timeout\": 30000,\n            \"title\": \"\",\n            \"type\": 0\n       "
        " }\n    }\n}\n";

tst_ActionJson::tst_ActionJson()
{
    QStandardPaths::setTestModeEnabled(true);
}

void tst_ActionJson::test_filePath()
{
    const QString jsonFilePath = ActionJson::jsonFilePath();
    QVERIFY(jsonFilePath.endsWith(QString("%1/actions.json").arg(qAppName())));
    QVERIFY(QFileInfo::exists(jsonFilePath));
}

void tst_ActionJson::test_load()
{
    const Action::Ptr action(new tst_Action(Action::Scope::User, KnownAction::Unknown, {}, TestId));

    QByteArray inputString(TestJson);
    QBuffer in(&inputString);
    in.open(QIODevice::ReadOnly);

    ActionStorage storage;
    ActionJson json(&storage);
    QCOMPARE(json.customActionIds(), {});

    json.load(&in);
    QCOMPARE(json.customActionIds(), { TestId.toString() });

    json.updateAction(action.get());
    QCOMPARE(action->app(), QStringLiteral("/usr/bin/ls"));
    QCOMPARE(action->args(), { QStringLiteral("-la") });

    json.popAction(action.get());
    QCOMPARE(json.customActionIds(), {});

    json.putAction(action.get());
    QCOMPARE(json.customActionIds(), { TestId.toString() });
}

void tst_ActionJson::test_save()
{
    const Action::Ptr action(new tst_Action(Action::Scope::User, KnownAction::Unknown, {}, TestId));
    action->setApp("/usr/bin/ls");
    action->setArgs({ "-la" });

    QByteArray outString;
    QBuffer out(&outString);
    out.open(QIODevice::WriteOnly);

    ActionStorage storage;
    ActionJson json(&storage);
    QCOMPARE(json.customActionIds(), {});

    json.putAction(action.get());
    QCOMPARE(json.customActionIds(), { TestId.toString() });

    json.popAction(action.get());
    QCOMPARE(json.customActionIds(), {});

    json.putAction(action.get());
    QCOMPARE(json.customActionIds(), { TestId.toString() });

    json.save(&out);

    QCOMPARE(outString, TestJson);
}
