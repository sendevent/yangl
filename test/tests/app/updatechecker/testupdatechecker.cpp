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

#include "app/updatechecker.h"
#include "version/versiontriplet.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

class MockReply : public QNetworkReply
{
    Q_OBJECT
public:
    MockReply(const QByteArray &data, QNetworkReply::NetworkError error, QObject *parent)
        : QNetworkReply(parent)
        , m_buffer(new QBuffer(this))
    {
        m_buffer->setData(data);
        m_buffer->open(QIODevice::ReadOnly);
        setError(error, error == QNetworkReply::NoError ? QString() : QStringLiteral("mock error"));
        setFinished(true);
        open(QIODevice::ReadOnly);
        QMetaObject::invokeMethod(this, &MockReply::finished, Qt::QueuedConnection);
    }

    void abort() override { }
    qint64 bytesAvailable() const override { return m_buffer->bytesAvailable() + QNetworkReply::bytesAvailable(); }

protected:
    qint64 readData(char *data, qint64 maxSize) override { return m_buffer->read(data, maxSize); }

private:
    QBuffer *m_buffer;
};

class MockNAM : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MockNAM(QObject *parent = {})
        : QNetworkAccessManager(parent)
    {
    }

    void setNextResponse(const QByteArray &body, QNetworkReply::NetworkError error = QNetworkReply::NoError)
    {
        m_nextBody = body;
        m_nextError = error;
        m_requestCount = 0;
    }

    int requestCount() const { return m_requestCount; }

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice * /*outgoing*/) override
    {
        Q_UNUSED(req)
        if (op == GetOperation) {
            ++m_requestCount;
        }
        return new MockReply(m_nextBody, m_nextError, this);
    }

private:
    QByteArray m_nextBody;
    QNetworkReply::NetworkError m_nextError { QNetworkReply::NoError };
    int m_requestCount { 0 };
};

class TestableUpdateChecker : public UpdateChecker
{
    Q_OBJECT
public:
    explicit TestableUpdateChecker(QNetworkAccessManager *nam, QObject *parent = {})
        : UpdateChecker(nam, parent)
    {
    }

protected:
    VersionTriplet currentAppVersion() const override
    {
        return VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_0_99_1);
    }
};

static QByteArray makeTagJson(const QString &tag)
{
    QJsonObject obj;
    obj[QStringLiteral("tag_name")] = tag;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

class TestUpdateChecker : public QObject
{
    Q_OBJECT

private slots:
    void test_update_detected();
    void test_no_update_same_version();
    void test_no_update_older_version();
    void test_v_prefix_stripped();
    void test_V_prefix_case_insensitive();
    void test_empty_tag_ignored();
    void test_network_error_suppressed();
    void test_no_overlapping();
    void test_apply_enabled_triggers();
    void test_apply_enabled_no_double_check();
    void test_pending_state_after_detection();
    void test_live_network();
};

void TestUpdateChecker::test_update_detected()
{
    const VersionTriplet base = VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_0_99_1);
    const VersionTriplet newer = VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_1_0_0);

    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(newer.toString()));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.count(), 1);

    const VersionTriplet emitted = VersionTriplet::fromString(spy.first().at(0).toString());
    QVERIFY(base < emitted);
}

void TestUpdateChecker::test_no_update_same_version()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_0_99_1).toString()));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 0);
}

void TestUpdateChecker::test_no_update_older_version()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(QStringLiteral("0.1.0")));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 0);
}

void TestUpdateChecker::test_v_prefix_stripped()
{
    const VersionTriplet v200 = VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_2_0_0);

    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson('v' + v200.toString()));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.first().at(0).toString(), v200.toString());
}

void TestUpdateChecker::test_V_prefix_case_insensitive()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(QStringLiteral("V3.1.0")));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.first().at(0).toString(), QStringLiteral("3.1.0"));
}

void TestUpdateChecker::test_empty_tag_ignored()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(QString()));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 0);
}

void TestUpdateChecker::test_network_error_suppressed()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(QByteArray(), QNetworkReply::ConnectionRefusedError);

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 0);
    QVERIFY(!checker.hasPendingUpdate());
}

void TestUpdateChecker::test_no_overlapping()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(QStringLiteral("5.0.0")));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.check(); // dispatches request, sets in-flight flag
    checker.check(); // must be a silent no-op
    checker.check();

    QVERIFY(spy.wait(3000));
    QCOMPARE(nam->requestCount(), 1);
    QCOMPARE(spy.count(), 1);
}

void TestUpdateChecker::test_apply_enabled_triggers()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_2_0_0).toString()));

    TestableUpdateChecker checker(nam);
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);

    checker.applyEnabled(true);
    QVERIFY(spy.wait(3000));
    QCOMPARE(spy.count(), 1);
}

void TestUpdateChecker::test_apply_enabled_no_double_check()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_2_0_0).toString()));

    TestableUpdateChecker checker(nam);

    checker.applyEnabled(true);
    QTest::qWait(500);
    const int countAfterFirst = nam->requestCount();

    checker.applyEnabled(true); // already enabled — must not fire another check
    QTest::qWait(500);
    QCOMPARE(nam->requestCount(), countAfterFirst);
}

void TestUpdateChecker::test_pending_state_after_detection()
{
    auto *nam = new MockNAM;
    nam->setNextResponse(makeTagJson(QStringLiteral("10.0.0")));

    TestableUpdateChecker checker(nam);
    QVERIFY(!checker.hasPendingUpdate());

    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.check();
    QVERIFY(spy.wait(3000));

    QVERIFY(checker.hasPendingUpdate());
    QCOMPARE(checker.pendingVersion(), QStringLiteral("10.0.0"));
    QCOMPARE(checker.pendingUrl(), UpdateChecker::RepoUrl);
}

void TestUpdateChecker::test_live_network()
{
    // Hits the real GitHub API. Skipped if the network is unavailable or no newer release  exists.
    // Uses a real QNetworkAccessManager while still forcing the in-code version to fake old one
    auto *realNam = new QNetworkAccessManager(this);
    TestableUpdateChecker checker(realNam, this);

    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.check();

    if (!spy.wait(15000)) {
        QSKIP("Live network test: no updateAvailable within 15 s "
              "(no network or no published release newer than 0.99.1)");
    }

    const QString emittedVersion = spy.first().at(0).toString();
    const VersionTriplet emitted = VersionTriplet::fromString(emittedVersion);
    const VersionTriplet base = VersionTriplet::fromKnown(VersionTriplet::KnownVersion::V_0_99_1);
    QVERIFY2(base < emitted,
             qPrintable(
                     QStringLiteral("Expected a version newer than %1, got: %2").arg(base.toString(), emittedVersion)));
}

QTEST_MAIN(TestUpdateChecker)
#include "testupdatechecker.moc"
