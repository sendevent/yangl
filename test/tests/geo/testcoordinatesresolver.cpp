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

    void test_location_real();
    void test_location_real_cases();
    void test_location_fake();

    void test_requestCoordinates_real_1();
    void test_requestCoordinates_real_3();

    void test_requestCoordinates_fake();

    void test_requestCoordinates_capital();
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
    m_resolver->ensureDataLoaded();
    QCOMPARE(m_resolver->m_data.size(), 241);
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
}

void TestCoordinatesResolver::test_location_real_cases()
{
    auto runCheck = [this](const auto &country, const auto &city) {
        auto checkResponse = [country, city](const auto &response) {
            qDebug() << country << city;
            QVERIFY(response.ok);
            QVERIFY(response.location.isValid());
            QCOMPARE(response.location.latitude(), 60.1708);
            QCOMPARE(response.location.longitude(), 24.9375);
        };

        const auto &response1 = m_resolver->lookupForPlace({
                country,
                city,
        });
        checkResponse(response1);

        const auto &response2 = m_resolver->lookupForPlace({
                country,
                city,
        });
        checkResponse(response2);
        QCOMPARE(response1, response2);
    };

    const QList<QPair<QString, QString>> data { { "Finland", "Helsinki" }, { "finland", "Helsinki" },
                                                { "Finland", "helsinki" }, { "finland", "helsinki" },
                                                { "Finland", "" },         { "finland", "" } };

    for (const auto &pair : data) {
        runCheck(pair.first, pair.second);
    }
}

void TestCoordinatesResolver::test_location_fake()
{
    auto checkResponse = [](const auto &response) {
        QVERIFY(!response.ok);
        QVERIFY(!response.location.isValid());
    };

    const auto &response1 = m_resolver->lookupForPlace({
            "Oz",
            "Emerald City",
    });
    checkResponse(response1);

    const auto &response2 = m_resolver->lookupForPlace({
            "Oz",
            "Emerald City",
    });
    checkResponse(response2);
    QCOMPARE(response1, response2);
}

const QList<QPair<QString, QString>> generateDataSet(const QString &country, const QString &city)
{
    /*{
        { "Country", "City" },
        { "country", "city" },
        { "country", "City" },
        { "Country", "city" },
        { "Country", "" },
        { "country", "" },
    }*/

    auto capitalize = [](const QString &value) {
        auto str(value);
        str[0] = str[0].toUpper();
        return str;
    };
    const auto &countryLower = country.toLower();
    const auto &cityLower = city.toLower();

    return {
        { capitalize(countryLower), capitalize(cityLower) },
        { countryLower, cityLower },
        { countryLower, capitalize(cityLower) },
        { capitalize(countryLower), cityLower },
        { capitalize(countryLower), "" },
        { countryLower, "" },
    };
}

void TestCoordinatesResolver::test_requestCoordinates_real_1()
{
    auto checkResult = [](const PlaceInfo &city, const auto idRequested, const auto idReceived) {
        QCOMPARE(idReceived, idRequested);
        QVERIFY(city.ok);
        QCOMPARE(city.country, "Finland");
        QCOMPARE(city.town, "Helsinki");
        QCOMPARE(city.location.latitude(), 60.1708);
        QCOMPARE(city.location.longitude(), 24.9375);
    };

    const QList<QPair<QString, QString>> &data = ::generateDataSet("Finland", "Helsinki");

    for (const auto &pair : data) {
        QSignalSpy spy(m_resolver, &CoordinatesResolver::coordinatesResolved);

        const auto idRequested = m_resolver->requestCoordinates({ pair.first, pair.second });

        spy.wait();
        QCOMPARE(spy.count(), 1);
        const QList<QVariant> &arguments = spy.takeFirst();

        QVERIFY(arguments.size() == 2);

        QVERIFY(arguments.at(0).typeId() == QMetaType::UInt);

        bool converted(false);
        const auto idReceived = arguments.at(0).toUInt(&converted);
        QVERIFY(converted);

        const auto &result = arguments.at(1).value<PlaceInfo>();

        checkResult(result, idRequested, idReceived);
    }
}

void TestCoordinatesResolver::test_requestCoordinates_real_3()
{

    auto checkResult = [](const PlaceInfo &placeRequested, const PlaceInfo &placeReceived, const auto idRequested,
                          const auto idReceived) {
        QCOMPARE(idReceived, idRequested);
        QVERIFY(placeReceived.ok);
        QCOMPARE(placeReceived.country.toLower(), placeRequested.country.toLower());
        if (placeRequested.town.isEmpty()) {
            QVERIFY(!placeReceived.town.isEmpty());
        } else {
            QCOMPARE(placeReceived.town.toLower(), placeRequested.town.toLower());
        }
    };

    const QList<QPair<QString, QString>> &data = ::generateDataSet("Finland", "Helsinki")
            + ::generateDataSet("Canada", "Ottawa") + ::generateDataSet("Australia", "Canberra");

    for (const auto &pair : data) {
        QSignalSpy spy(m_resolver, &CoordinatesResolver::coordinatesResolved);

        const PlaceInfo placeRequested = { pair.first, pair.second };
        const auto idRequested = m_resolver->requestCoordinates(placeRequested);

        spy.wait();
        QCOMPARE(spy.count(), 1);
        const QList<QVariant> &arguments = spy.takeFirst();

        QVERIFY(arguments.size() == 2);

        QVERIFY(arguments.at(0).typeId() == QMetaType::UInt);

        bool converted(false);
        const auto idReceived = arguments.at(0).toUInt(&converted);
        QVERIFY(converted);

        const auto &placeReceived = arguments.at(1).value<PlaceInfo>();

        checkResult(placeRequested, placeReceived, idRequested, idReceived);
    }
}

void TestCoordinatesResolver::test_requestCoordinates_fake()
{
    CoordinatesResolver resolver;

    auto checkResult = [](const PlaceInfo &city, const auto idRequested, const auto idReceived) {
        QCOMPARE(idReceived, idRequested);
        QVERIFY(!city.ok);
        QCOMPARE(city.country, "Oz");
        QCOMPARE(city.town, "Emerald City");
        QVERIFY(!city.location.isValid());
    };

    const QList<QPair<QString, QString>> &data = ::generateDataSet("Oz", "Emerald City");

    {
        QSignalSpy spy(&resolver, &CoordinatesResolver::coordinatesResolved);

        const auto idRequested = resolver.requestCoordinates({ "Oz", "Emerald City" });

        spy.wait();
        QCOMPARE(spy.count(), 1);
        const QList<QVariant> &arguments = spy.takeFirst();

        QVERIFY(arguments.size() == 2);

        QVERIFY(arguments.at(0).typeId() == QMetaType::UInt);

        bool converted(false);
        const auto idReceived = arguments.at(0).toUInt(&converted);
        QVERIFY(converted);

        const auto &result = arguments.at(1).value<PlaceInfo>();

        checkResult(result, idRequested, idReceived);
    }

    {
        QSignalSpy spy(&resolver, &CoordinatesResolver::coordinatesResolved);

        const auto idRequested = resolver.requestCoordinates("Oz", "Emerald City");

        spy.wait();
        QCOMPARE(spy.count(), 1);
        const QList<QVariant> &arguments = spy.takeFirst();

        QVERIFY(arguments.size() == 2);

        QVERIFY(arguments.at(0).typeId() == QMetaType::UInt);

        bool converted(false);
        const auto idReceived = arguments.at(0).toUInt(&converted);
        QVERIFY(converted);

        const auto &result = arguments.at(1).value<PlaceInfo>();

        checkResult(result, idRequested, idReceived);
    }
}

void TestCoordinatesResolver::test_requestCoordinates_capital()
{
    auto performCheck = [this](const QString &country, const QString &city) {
        auto checkResponse = [](const auto &response) {
            QVERIFY(response.ok);
            QVERIFY(response.capital);
            QVERIFY(response.location.isValid());
        };

        const auto &response1 = m_resolver->lookupForPlace({
                country,
                "",
        });
        checkResponse(response1);
        QCOMPARE(response1.town, city);

        const auto &response2 = m_resolver->lookupForPlace({
                country,
                "",
        });
        checkResponse(response2);
        QCOMPARE(response2.town, city);
        QCOMPARE(response1, response2);
    };

    const QList<QPair<QString, QString>> regions {
        { "United States", "Washington" },
        { "Finland", "Helsinki" },
        { "Vatican City", "Vatican City" },
    };

    for (const auto &region : regions) {
        performCheck(region.first, region.second);
    }
}

QTEST_MAIN(TestCoordinatesResolver)
#include "testcoordinatesresolver.moc"
