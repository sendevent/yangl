#pragma once

#include <QSignalSpy>
#include <QRegularExpression>
#include <QVariant>
#include <QString>
#include <QTest>

namespace testutils {

inline constexpr int kShortWaitMs = 500;
inline constexpr int kDefaultSpyWaitMs = 3000;
inline constexpr int kLiveNetworkWaitMs = 15000;

inline void ignoreWarning(const QString &body)
{
    const QString rx = QStringLiteral(".*") + QRegularExpression::escape(body) + QStringLiteral(".*");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(rx));
}

inline void waitForSpyOrFail(QSignalSpy &spy, int timeoutMs = kDefaultSpyWaitMs)
{
    if (spy.isEmpty()) {
        QVERIFY(spy.wait(timeoutMs));
    }
}

inline void expectNoSignal(QSignalSpy &spy, int quietMs = kShortWaitMs)
{
    QTest::qWait(quietMs);
    QCOMPARE(spy.count(), 0);
}

inline QList<QVariant> takeFirstArgsOrFail(QSignalSpy &spy, int expectedCount = -1)
{
    if (spy.isEmpty()) {
        QTest::qFail("Expected at least one signal in QSignalSpy, but it is empty", __FILE__, __LINE__);
        return {};
    }

    const QList<QVariant> args = spy.takeFirst();

    if (expectedCount >= 0) {
        if (args.size() != expectedCount) {
            QTest::qFail("QSignalSpy first signal argument count mismatch", __FILE__, __LINE__);
            return {};
        }
    }

    return args;
}

} // namespace testutils
