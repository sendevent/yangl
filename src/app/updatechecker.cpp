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

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
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

    if (reply->error() != QNetworkReply::NoError) {
        // 404 = no releases published yet; not worth logging as a warning
        if (reply->error() != QNetworkReply::ContentNotFoundError) {
            WRN << "Update check failed:" << reply->errorString() << reply->error();
        }
        return;
    }

    const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    QString tag = json[QStringLiteral("tag_name")].toString().trimmed();
    if (tag.startsWith('v', Qt::CaseInsensitive)) {
        tag.remove(0, 1);
    }

    if (tag.isEmpty()) {
        WRN << "Update check: empty tag_name in response";
        return;
    }

    const VersionTriplet latest = VersionTriplet::fromString(tag);
    const VersionTriplet current(yangl::V.Major, yangl::V.Minor, yangl::V.Patch);

    LOG << "Update check: current" << current.toString() << "latest" << latest.toString();

    if (current < latest) {
        m_pendingVersion = latest.toString();
        m_pendingUrl = RepoUrl;
        emit updateAvailable(m_pendingVersion, m_pendingUrl);
    }
}
