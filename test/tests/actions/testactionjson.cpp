/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/actionjson.h"
#include "actions/actionstorage.h"
#include "testaction.h"
#include "testutils.h"

#include <QBuffer>
#include <QTest>

class TestActionJson : public QObject
{
    Q_OBJECT
public:
    explicit TestActionJson();

signals:

private slots:
    void test_filePath();
    void test_load();
    void test_save();
    void test_load_invalidJson();
    void test_load_emptyInput();
    void test_load_nullDevice();
    void test_tryLoad_invalidJson_errorCode();
    void test_tryLoad_emptyInput_errorCode();
    void test_tryLoad_nullDevice_errorCode();

private:
    static const Action::Id TestId;
    static const QByteArray TestJson;
};

/*static*/ const Action::Id TestActionJson::TestId = Action::Id("{057c11c0-de87-48c4-a98e-93471b4290ca}");

/*static*/ const QByteArray TestActionJson::TestJson =
        "{\n    \"custom\": {\n        \"{057c11c0-de87-48c4-a98e-93471b4290ca}\": {\n            \"anchor\": 0,\n     "
        "       \"app\": \"/usr/bin/ls\",\n            \"args\": [\n                \"-la\"\n            ],\n          "
        "  \"forcedDisplay\": false,\n            \"id\": \"{057c11c0-de87-48c4-a98e-93471b4290ca}\",\n            "
        "\"scope\": 2,\n            \"timeout\": 30,\n            \"title\": \"\",\n            \"toggleGroup\": "
        "\"\",\n            \"toggleOn\": false,\n            \"type\": 0\n        }\n    }\n}\n";

TestActionJson::TestActionJson() { }

void TestActionJson::test_filePath()
{
    const QString jsonFilePath = ActionJson::jsonFilePath();
    QVERIFY(jsonFilePath.endsWith(QString("%1/actions.json").arg(qAppName())));
}

void TestActionJson::test_load()
{
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown, {}, TestId));

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

void TestActionJson::test_save()
{
    const Action::Ptr action(new TestAction(Action::Flow::Custom, Action::NordVPN::Unknown, {}, TestId));
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

void TestActionJson::test_load_invalidJson()
{
    testutils::ignoreWarning(QStringLiteral("error parsing document: unterminated object"));
    QByteArray garbage("{ this is not valid json !@#$ }");
    QBuffer in(&garbage);
    in.open(QIODevice::ReadOnly);

    ActionStorage storage;
    ActionJson json(&storage);

    QCOMPARE(json.load(&in), false);
    QCOMPARE(json.customActionIds(), {});
}

void TestActionJson::test_load_emptyInput()
{
    testutils::ignoreWarning(QStringLiteral("No JSON to load"));

    QByteArray empty;
    QBuffer in(&empty);
    in.open(QIODevice::ReadOnly);

    ActionStorage storage;
    ActionJson json(&storage);

    QCOMPARE(json.load(&in), false);
    QCOMPARE(json.customActionIds(), {});
}

void TestActionJson::test_load_nullDevice()
{
    testutils::ignoreWarning(QStringLiteral("Input device is null or not readable"));
    ActionStorage storage;
    ActionJson json(&storage);

    QCOMPARE(json.load(static_cast<QIODevice *>(nullptr)), false);
    QCOMPARE(json.customActionIds(), {});
}

void TestActionJson::test_tryLoad_invalidJson_errorCode()
{
    QByteArray garbage("{ this is not valid json !@#$ }");
    QBuffer in(&garbage);
    in.open(QIODevice::ReadOnly);

    ActionStorage storage;
    ActionJson json(&storage);

    const auto parsed = json.tryLoad(&in);
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error().code, ActionJson::LoadErrorCode::InvalidJson);
    QVERIFY(parsed.error().details.contains(QStringLiteral("error parsing document:")));
}

void TestActionJson::test_tryLoad_emptyInput_errorCode()
{
    QByteArray empty;
    QBuffer in(&empty);
    in.open(QIODevice::ReadOnly);

    ActionStorage storage;
    ActionJson json(&storage);

    const auto parsed = json.tryLoad(&in);
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error().code, ActionJson::LoadErrorCode::EmptyInput);
    QCOMPARE(parsed.error().details, QStringLiteral("No JSON to load"));
}

void TestActionJson::test_tryLoad_nullDevice_errorCode()
{
    ActionStorage storage;
    ActionJson json(&storage);

    const auto parsed = json.tryLoad(static_cast<QIODevice *>(nullptr));
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error().code, ActionJson::LoadErrorCode::InvalidDevice);
}

QTEST_MAIN(TestActionJson)
#include "testactionjson.moc"
