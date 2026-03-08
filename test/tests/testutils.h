#pragma once

#include <QSignalSpy>
#include <QRegularExpression>
#include <QVariant>
#include <QString>
#include <QTest>
#include <chrono>

namespace testutils {

inline constexpr std::chrono::milliseconds kShortWait = std::chrono::milliseconds(500);
inline constexpr int kShortWaitMs = static_cast<int>(kShortWait.count());
inline constexpr std::chrono::milliseconds kDefaultSpyWait = std::chrono::seconds(3);
inline constexpr int kDefaultSpyWaitMs = static_cast<int>(kDefaultSpyWait.count());
inline constexpr std::chrono::milliseconds kLiveNetworkWait = std::chrono::seconds(15);
inline constexpr int kLiveNetworkWaitMs = static_cast<int>(kLiveNetworkWait.count());

inline void ignoreWarning(const QString &body)
{
    const QString rx = QStringLiteral(".*") + QRegularExpression::escape(body) + QStringLiteral(".*");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(rx));
}

inline void waitForSpyOrFail(QSignalSpy &spy, std::chrono::milliseconds timeout = kDefaultSpyWait)
{
    if (spy.isEmpty()) {
        QVERIFY(spy.wait(static_cast<int>(timeout.count())));
    }
}

inline void waitForSpyOrFail(QSignalSpy &spy, int timeoutMs)
{
    waitForSpyOrFail(spy, std::chrono::milliseconds(timeoutMs));
}

inline void expectNoSignal(QSignalSpy &spy, std::chrono::milliseconds quiet = kShortWait)
{
    QTest::qWait(static_cast<int>(quiet.count()));
    QCOMPARE(spy.count(), 0);
}

inline void expectNoSignal(QSignalSpy &spy, int quietMs)
{
    expectNoSignal(spy, std::chrono::milliseconds(quietMs));
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
