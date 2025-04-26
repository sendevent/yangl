/*
   Copyright (C) 2025 Denis Gofman - <sendevent@gmail.com>

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

#include "geo/coordinatesresolver.h"

#include <QSignalSpy>
#include <QTest>
#include <qtestcase.h>

class TestCoordinatesResolver : public QObject
{
    Q_OBJECT
private:
    CoordinatesResolver *m_resolver { nullptr };
private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_loadDataBuiltin();
    void test_loadDataDynamic();

    void test_location_real();
    void test_location_fake();

    void test_requestCoordinates_real();
    void test_requestCoordinates_fake();
};

void TestCoordinatesResolver::initTestCase()
{
    m_resolver = new CoordinatesResolver();
}

void TestCoordinatesResolver::cleanupTestCase()
{
    delete m_resolver;
}

void TestCoordinatesResolver::test_loadDataBuiltin()
{
    QVERIFY(m_resolver->loadDataBuiltin());
    QCOMPARE(m_resolver->m_data.size(), 241);
}

void TestCoordinatesResolver::test_loadDataDynamic()
{
    QEXPECT_FAIL("", "TBD", Continue);
    QFAIL("Not implemented yet");
}

void TestCoordinatesResolver::test_location_real()
{
    auto checkResponse = [](const auto &response) {
        QVERIFY(response.ok);
        QVERIFY(response.location.isValid());
        QCOMPARE(response.location.latitude(), 60.1708);
        QCOMPARE(response.location.longitude(), 24.9375);
    };

    const auto &response1 = m_resolver->lookupForPlace({
            "Finland",
            "Helsinki",
    });
    checkResponse(response1);

    const auto &response2 = m_resolver->lookupForPlace({
            "Finland",
            "Helsinki",
    });
    checkResponse(response2);
    QCOMPARE(response1, response2);

    const auto &response3 = m_resolver->lookupForPlace("Finland", "Helsinki");
    checkResponse(response3);
    QCOMPARE(response2, response3);
}

void TestCoordinatesResolver::test_location_fake()
{
    auto checkResponse = [](const auto &response) {
        QVERIFY(!response.ok);
        QVERIFY(!response.location.isValid());
    };

    const auto &response1 = m_resolver->lookupForPlace({
            "Russia",
            "Mariupol",
    });
    checkResponse(response1);

    const auto &response2 = m_resolver->lookupForPlace({
            "Russia",
            "Mariupol",
    });
    checkResponse(response2);
    QCOMPARE(response1, response2);

    const auto &response3 = m_resolver->lookupForPlace("Russia", "Mariupol");
    checkResponse(response3);
    QCOMPARE(response2, response3);
}

void TestCoordinatesResolver::test_requestCoordinates_real()
{
    CoordinatesResolver resolver;

    bool finished(false);
    auto onResolved = [&finished](const PlaceInfo &city) {
        finished = true;

        QVERIFY(city.ok);
        QCOMPARE(city.country, "Finland");
        QCOMPARE(city.town, "Helsinki");
        QCOMPARE(city.location.latitude(), 60.1708);
        QCOMPARE(city.location.longitude(), 24.9375);
    };
    connect(&resolver, &CoordinatesResolver::coordinatesResoloved, this, onResolved);

    resolver.requestCoordinates({
            "Finland",
            "Helsinki",
    });

    resolver.requestCoordinates("Finland", "Helsinki");
}

void TestCoordinatesResolver::test_requestCoordinates_fake()
{
    CoordinatesResolver resolver;

    bool finished(false);
    auto onResolved = [&finished](const PlaceInfo &city) {
        finished = true;

        QVERIFY(!city.ok);
        QCOMPARE(city.country, "Russia");
        QCOMPARE(city.town, "Mariupol");
    };
    connect(&resolver, &CoordinatesResolver::coordinatesResoloved, this, onResolved);

    resolver.requestCoordinates({
            "Russia",
            "Mariupol",
    });

    resolver.requestCoordinates("Russia", "Mariupol");
}

QTEST_MAIN(TestCoordinatesResolver)
#include "testcoordinatesresolver.moc"
