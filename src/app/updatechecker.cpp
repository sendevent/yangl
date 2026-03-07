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

#include "updatechecker.h"

#include "app/common.h"
#include "version/appversiondefs.h"
#include "version/versiontriplet.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

/*static*/ const QUrl UpdateChecker::RepoUrl(QStringLiteral("https://github.com/sendevent/yangl"));

/*static*/ QString UpdateChecker::errorCodeToString(ResponseParsingError code)
{
    switch (code) {
    case NetworkError:
        return QStringLiteral("NetworkError");
    case InvalidJson:
        return QStringLiteral("InvalidJson");
    case MissingTagName:
        return QStringLiteral("MissingTagName");
    case EmptyTagName:
        return QStringLiteral("EmptyTagName");
    case InvalidVersionTag:
        return QStringLiteral("InvalidVersionTag");
    case ResponseParsingErrorCount:
        return QStringLiteral("ResponseParsingErrorCount");
    }
    return QStringLiteral("UnknownResponseParsingError");
}

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

UpdateChecker::UpdateChecker(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
}

VersionTriplet UpdateChecker::currentAppVersion() const
{
    return { yangl::V.Major, yangl::V.Minor, yangl::V.Patch };
}

void UpdateChecker::check()
{
    if (m_inFlight) {
        return;
    }
    m_inFlight = true;
    static const QUrl kReleasesApiUrl(QStringLiteral("https://api.github.com/repos/sendevent/yangl/releases/latest"));
    QNetworkRequest req(kReleasesApiUrl);
    req.setRawHeader("Accept", "application/vnd.github+json");
    auto *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReplyFinished(reply); });
    LOG << "Checking for updates...";
}

void UpdateChecker::applyEnabled(bool enabled)
{
    if (enabled && !m_enabled) {
        check();
    }
    m_enabled = enabled;
}

void UpdateChecker::onReplyFinished(QNetworkReply *reply)
{
    m_inFlight = false;
    reply->deleteLater();

    const auto parsed = parseResponse(reply);
    if (!parsed) {
        const auto &error = parsed.error();
        if (!error.details.isEmpty()) {
            WRN << errorCodeToString(error.code) << error.details;
        }
        return;
    }

    const auto latest = parsed.value();
    const VersionTriplet current = currentAppVersion();

    LOG << "Update check: current" << current.toString() << "latest" << latest.toString();

    if (current < latest) {
        m_pendingVersion = latest.toString();
        m_pendingUrl = RepoUrl;
        emit updateAvailable(m_pendingVersion, m_pendingUrl);
    }
}

UpdateChecker::ParseResult UpdateChecker::parseResponse(QNetworkReply *reply)
{
    if (!reply) {
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::NetworkError,
                                        QStringLiteral("Update check: Invalid reply instance") };
        return std::unexpected(res);
    }

    if (reply->error() != QNetworkReply::NoError) {
        const QString message =
                QStringLiteral("Update check failed: '%1' '%2'").arg(reply->errorString()).arg(reply->error());
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::NetworkError, message };
        return std::unexpected(res);
    }

    static const QString tagName = QStringLiteral("tag_name");
    QJsonParseError jpe;
    const QByteArray data = reply->readAll();
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jpe);
    if (jpe.error != QJsonParseError::NoError) {
        const QString message = QStringLiteral("Update check: JSON parsing error: '%1' %2 '%3'")
                                        .arg(jpe.errorString(), QString::number(jpe.offset), QString::fromUtf8(data));
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::InvalidJson, message };
        return std::unexpected(res);
    }

    const QJsonObject json = jsonDoc.object();
    if (json.isEmpty()) {
        const QString message = QStringLiteral("Update check: Received JSON is empty");
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::InvalidJson, message };
        return std::unexpected(res);
    }
    if (!json.contains(tagName)) {
        const QString message = QStringLiteral("Update check: JSON tag not found: '%1' '%2'")
                                        .arg(tagName, QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact)));
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::MissingTagName, message };
        return std::unexpected(res);
    }
    QString tag = json[tagName].toString().trimmed();
    if (tag.startsWith('v', Qt::CaseInsensitive)) {
        tag.remove(0, 1);
    }

    if (tag.isEmpty()) {
        const QString message = QStringLiteral("Update check: empty %1 in response").arg(tagName);
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::EmptyTagName, message };
        return std::unexpected(res);
    }

    const auto parsedVersion = VersionTriplet::fromString(tag);
    if (!parsedVersion) {
        const QString message =
                QStringLiteral("Update check: invalid version format: '%1' (%2)")
                        .arg(tag, VersionTriplet::errorCodeToString(parsedVersion.error()));
        const ResponseParseResult res { UpdateChecker::ResponseParsingError::InvalidVersionTag, message };
        return std::unexpected(res);
    }

    return parsedVersion.value();
}
