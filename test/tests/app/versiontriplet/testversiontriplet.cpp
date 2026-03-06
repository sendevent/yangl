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

#include "version/versiontriplet.h"

#include <QTest>

class TestVersionTriplet : public QObject
{
    Q_OBJECT

private slots:
    void test_tryFromString_emptyInput();
    void test_tryFromString_wrongPartsCount();
    void test_tryFromString_emptyComponent();
    void test_tryFromString_invalidComponent();
    void test_tryFromString_validWithSpaces();
};

void TestVersionTriplet::test_tryFromString_emptyInput()
{
    const auto parsed = VersionTriplet::fromString(QString());
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error(), VersionTriplet::ParseError::EmptyInput);
}

void TestVersionTriplet::test_tryFromString_wrongPartsCount()
{
    const auto parsed = VersionTriplet::fromString(QStringLiteral("1.2"));
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error(), VersionTriplet::ParseError::WrongPartsCount);
}

void TestVersionTriplet::test_tryFromString_emptyComponent()
{
    const auto parsed = VersionTriplet::fromString(QStringLiteral("1..3"));
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error(), VersionTriplet::ParseError::EmptyComponent);
}

void TestVersionTriplet::test_tryFromString_invalidComponent()
{
    const auto parsed = VersionTriplet::fromString(QStringLiteral("1.a.3"));
    QVERIFY(!parsed.has_value());
    QCOMPARE(parsed.error(), VersionTriplet::ParseError::InvalidComponent);
}

void TestVersionTriplet::test_tryFromString_validWithSpaces()
{
    const auto parsed = VersionTriplet::fromString(QStringLiteral(" 1.2.3 "));
    QVERIFY(parsed.has_value());
    QCOMPARE(parsed->major(), 1);
    QCOMPARE(parsed->minor(), 2);
    QCOMPARE(parsed->patch(), 3);
}

QTEST_MAIN(TestVersionTriplet)
#include "testversiontriplet.moc"
