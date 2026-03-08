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

#pragma once

#include "version/versiontriplet.h"

#include <QObject>
#include <QUrl>
#include <expected>

class QNetworkAccessManager;
class QNetworkReply;

/*!
 * \brief Asynchronous GitHub release checker.
 *
 * Queries the GitHub Releases API for the latest published tag and compares
 * it against the running build version via \c currentAppVersion(). If a newer
 * version is found, \c updateAvailable is emitted and the result is cached so
 * that widgets created after the check can still call \c hasPendingUpdate(),
 * \c pendingVersion(), and \c pendingUrl() without waiting for another round.
 *
 * An in-flight guard prevents overlapping requests. Network/parse errors are
 * ignored for control flow (no update signal is emitted) and logged as warnings.
 *
 * \c currentAppVersion() is virtual to allow test subclasses to inject an
 * arbitrary baseline version.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    enum class ResponseParsingError
    {
        NetworkError,
        InvalidJson,
        MissingTagName,
        EmptyTagName,
        InvalidVersionTag,
        ResponseParsingErrorCount
    };

    struct ResponseParseResult {
        ResponseParsingError code;
        QString details;
    };
    static QString errorCodeToString(ResponseParsingError code);

    explicit UpdateChecker(QObject *parent = {});
    void check();
    void applyEnabled(bool enabled);

    static const QUrl RepoUrl;

    bool hasPendingUpdate() const { return !m_pendingVersion.isEmpty(); }
    QString pendingVersion() const { return m_pendingVersion; }
    QUrl pendingUrl() const { return m_pendingUrl; }

signals:
    void updateAvailable(const QString &version, const QUrl &repoUrl);

protected:
    explicit UpdateChecker(QNetworkAccessManager *nam, QObject *parent = {});
    virtual VersionTriplet currentAppVersion() const;

private:
    void onReplyFinished(QNetworkReply *reply);
    QNetworkAccessManager *m_nam { nullptr };
    bool m_inFlight { false };
    bool m_enabled { false };
    QString m_pendingVersion;
    QUrl m_pendingUrl;

    using ParseResult = std::expected<VersionTriplet, ResponseParseResult>;
    [[nodiscard]] ParseResult parseResponse(QNetworkReply *reply);
};
