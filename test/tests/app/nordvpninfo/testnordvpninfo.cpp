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
    void test_statusToText_loopCoverage();
    void test_allStatuses_coversEnumRangeWithoutSentinel();
    void test_uptimeParseErrorCodeToString();
    void test_statusParseErrorCodeToString();
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

void TestNordVpnInfo::test_statusToText_loopCoverage()
{
    const auto expectedText = [](NordVpnInfo::Status status) -> QString {
        switch (status) {
        case NordVpnInfo::Status::Unknown:
            return QStringLiteral("Unknown");
        case NordVpnInfo::Status::Disconnected:
            return QStringLiteral("Disconnected");
        case NordVpnInfo::Status::Connecting:
            return QStringLiteral("Connecting");
        case NordVpnInfo::Status::Connected:
            return QStringLiteral("Connected");
        case NordVpnInfo::Status::Disconnecting:
            return QStringLiteral("Disconnecting");
        case NordVpnInfo::Status::StatusCount:
            return QStringLiteral("StatusCount");
        }
        return QStringLiteral("UnexpectedStatus");
    };

    for (int i = 0; i <= static_cast<int>(NordVpnInfo::Status::StatusCount); ++i) {
        const auto status = static_cast<NordVpnInfo::Status>(i);
        QCOMPARE(NordVpnInfo::statusToText(status), expectedText(status));
    }
}

void TestNordVpnInfo::test_allStatuses_coversEnumRangeWithoutSentinel()
{
    const auto all = NordVpnInfo::allStatuses();
    QCOMPARE(all.size(), static_cast<int>(NordVpnInfo::Status::StatusCount));
    QVERIFY(!all.contains(NordVpnInfo::Status::StatusCount));

    for (int i = 0; i < static_cast<int>(NordVpnInfo::Status::StatusCount); ++i) {
        QVERIFY(all.contains(static_cast<NordVpnInfo::Status>(i)));
    }
}

void TestNordVpnInfo::test_uptimeParseErrorCodeToString()
{
    const auto expectedText = [](NordVpnInfo::UptimeParseError code) -> QString {
        switch (code) {
        case NordVpnInfo::UptimeParseError::EmptyInput:
            return QStringLiteral("EmptyInput");
        case NordVpnInfo::UptimeParseError::InvalidToken:
            return QStringLiteral("InvalidToken");
        case NordVpnInfo::UptimeParseError::UptimeParseErrorCount:
            return QStringLiteral("UptimeParseErrorCount");
        }
        return QStringLiteral("UnexpectedUptimeParseError");
    };

    for (int i = 0; i <= static_cast<int>(NordVpnInfo::UptimeParseError::UptimeParseErrorCount); ++i) {
        const auto code = static_cast<NordVpnInfo::UptimeParseError>(i);
        QCOMPARE(NordVpnInfo::errorCodeToString(code), expectedText(code));
    }
}

void TestNordVpnInfo::test_statusParseErrorCodeToString()
{
    const auto expectedText = [](NordVpnInfo::StatusParseErrorCode code) -> QString {
        switch (code) {
        case NordVpnInfo::StatusParseErrorCode::EmptyInput:
            return QStringLiteral("EmptyInput");
        case NordVpnInfo::StatusParseErrorCode::MalformedLine:
            return QStringLiteral("MalformedLine");
        case NordVpnInfo::StatusParseErrorCode::MissingStatus:
            return QStringLiteral("MissingStatus");
        case NordVpnInfo::StatusParseErrorCode::InvalidStatus:
            return QStringLiteral("InvalidStatus");
        case NordVpnInfo::StatusParseErrorCode::InvalidUptime:
            return QStringLiteral("InvalidUptime");
        case NordVpnInfo::StatusParseErrorCode::StatusParseErrorCodeCount:
            return QStringLiteral("StatusParseErrorCodeCount");
        }
        return QStringLiteral("UnexpectedStatusParseErrorCode");
    };

    for (int i = 0; i <= static_cast<int>(NordVpnInfo::StatusParseErrorCode::StatusParseErrorCodeCount); ++i) {
        const auto code = static_cast<NordVpnInfo::StatusParseErrorCode>(i);
        QCOMPARE(NordVpnInfo::errorCodeToString(code), expectedText(code));
    }
}

QTEST_MAIN(TestNordVpnInfo)
#include "testnordvpninfo.moc"
