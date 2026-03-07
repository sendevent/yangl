#pragma once

#include <QRegularExpression>
#include <QString>
#include <QTest>

namespace testutils {

inline void ignoreWarning(const QString &body)
{
    const QString rx = QStringLiteral(".*") + QRegularExpression::escape(body) + QStringLiteral(".*");
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(rx));
}

} // namespace testutils
