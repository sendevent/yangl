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

#include "app/nordvpninfo.h"
#include "testutils.h"

#include <QObject>
#include <QTest>

class TestNordVpnInfo : public QObject
{
    Q_OBJECT

private slots:
    void test_fromString_emptyInput();
    void test_fromString_malformedLine();
    void test_fromString_missingStatus();
    void test_fromString_invalidStatus();
    void test_fromString_invalidUptime();
    void test_fromString_valid();
};

void TestNordVpnInfo::test_fromString_emptyInput()
{
    testutils::ignoreWarning(QStringLiteral("Input is empty"));

    const NordVpnInfo parsed = NordVpnInfo::fromString({});
    QCOMPARE(parsed, NordVpnInfo {});
}

void TestNordVpnInfo::test_fromString_malformedLine()
{
    testutils::ignoreWarning(QStringLiteral("MalformedLine No ':' separator in line: 'Status Connected'"));
    const NordVpnInfo parsed = NordVpnInfo::fromString("Status Connected\nServer: srv");
    QCOMPARE(parsed, NordVpnInfo {});
}

void TestNordVpnInfo::test_fromString_missingStatus()
{
    testutils::ignoreWarning(QStringLiteral("No status field found in input"));
    const NordVpnInfo parsed = NordVpnInfo::fromString("Server: srv\nCountry: Country");
    QCOMPARE(parsed, NordVpnInfo {});
}

void TestNordVpnInfo::test_fromString_invalidStatus()
{
    testutils::ignoreWarning(QStringLiteral("Unrecognized status value: 'TotallyInvalid'"));
    const NordVpnInfo parsed = NordVpnInfo::fromString("Status: TotallyInvalid\nServer: srv");
    QCOMPARE(parsed, NordVpnInfo {});
}

void TestNordVpnInfo::test_fromString_invalidUptime()
{
    testutils::ignoreWarning(QStringLiteral("Failed parsing uptime 'nope tokens': InvalidToken"));
    const QString input = "Status: Connected\nServer: srv\nUptime: nope tokens";
    const NordVpnInfo parsed = NordVpnInfo::fromString(input);
    QCOMPARE(parsed, NordVpnInfo {});
}

void TestNordVpnInfo::test_fromString_valid()
{
    const QString input =
            "Status: Connected\nServer: test.server\nCountry: Neverland\nCity: TestCity\nUptime: 1 day 2 hours";
    const NordVpnInfo parsed = NordVpnInfo::fromString(input);

    QCOMPARE(parsed.status(), NordVpnInfo::Status::Connected);
    QCOMPARE(parsed.server(), QString("test.server"));
    QCOMPARE(parsed.country(), QString("Neverland"));
    QCOMPARE(parsed.city(), QString("TestCity"));
}

QTEST_MAIN(TestNordVpnInfo)
#include "testnordvpninfo.moc"
