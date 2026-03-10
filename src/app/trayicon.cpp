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

#include "trayicon.h"

#include "app/common.h"
#include "settings/appsettings.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>
#include <QTextDocumentFragment>
#include <chrono>

/*static*/ QMap<NordVpnInfo::Status, TrayIcon::IconInfo> TrayIcon::m_allIcons = {};
/*static*/ QMap<NordVpnInfo::Status, QIcon> TrayIcon::m_composedIcons = {};

/*static*/ void TrayIcon::reloadIcons()
{
    m_allIcons.clear();
    m_composedIcons.clear();

    for (const auto state : NordVpnInfo::allStatuses()) {
        IconInfo info;
        info.m_status = state;

        switch (state) {
        case NordVpnInfo::Status::Connected:
            info.m_base = AppSettings::Tray->IcnConnected->read().toString();
            info.m_sub = AppSettings::Tray->IcnConnectedSub->read().toString();
            break;
        case NordVpnInfo::Status::Disconnected:
            info.m_base = AppSettings::Tray->IcnDisconnected->read().toString();
            info.m_sub = AppSettings::Tray->IcnDisconnectedSub->read().toString();
            break;
        case NordVpnInfo::Status::Connecting:
        case NordVpnInfo::Status::Disconnecting:
            info.m_base = AppSettings::Tray->IcnConnecting->read().toString();
            info.m_sub = AppSettings::Tray->IcnConnectingSub->read().toString();
            break;
        default:
            info.m_base = AppSettings::Tray->IcnUnknown->read().toString();
            info.m_sub = AppSettings::Tray->IcnUnknownSub->read().toString();
            break;
        }
        m_allIcons.insert(info.m_status, info);
        m_composedIcons.insert(state, generateIcon(state));
    }
}

/*static*/ TrayIcon::IconInfo TrayIcon::infoPixmaps(const NordVpnInfo::Status forStatus)
{
    return m_allIcons.value(forStatus);
}

/*static*/ QIcon TrayIcon::generateIcon(const NordVpnInfo::Status forStatus)
{
    const IconInfo &info = infoPixmaps(forStatus);
    QPixmap base(info.m_base);

    if (!info.m_sub.isEmpty()) {
        const QRect baseRect(base.rect());
        const QPixmap &sub = QPixmap(info.m_sub)
                                     .scaled(baseRect.width() / 2, baseRect.height() / 2, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        const QRect &subRect = sub.rect();
        QRect targetRect(subRect);
        targetRect.moveTopLeft(base.rect().center());

        QPainter p(&base);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setRenderHint(QPainter::LosslessImageRendering);
        p.drawPixmap(targetRect, sub, subRect);
    }
    return QIcon(base);
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_isFirstChange(true)
    , m_duration(std::chrono::seconds(5))
{
    deployDefaults();
    reloadIcons();

    const auto &icon = iconForStatus(NordVpnInfo::Status::Unknown);
    updateStateText(textForState(NordVpnInfo()), icon);
    setIcon(icon);
}

/*static*/ QIcon TrayIcon::iconForState(const NordVpnInfo &state)
{
    return iconForStatus(state.status());
}

/*static*/ QIcon TrayIcon::iconForStatus(const NordVpnInfo::Status &status)
{
    return m_composedIcons[status];
}

void TrayIcon::setMessageDuration(int durationSecs)
{
    setMessageDuration(std::chrono::seconds(durationSecs));
}

void TrayIcon::setMessageDuration(std::chrono::seconds duration)
{
    m_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

void TrayIcon::updateIcon(NordVpnInfo::Status status)
{
    const QIcon &icn = iconForStatus(status);
    if (!icn.isNull()) {
        setIcon(icn);
    }
}

void TrayIcon::setState(const NordVpnInfo &state)
{
    const auto &stateText = textForState(state);
    bool showPopup = false;

    if (m_state.status() != state.status() && !qApp->isSavingSession()) {
        updateIcon(state.status());
        showPopup = true;

        if (m_isFirstChange && state.status() == NordVpnInfo::Status::Connected) {
            if (AppSettings::Monitor->Active->read().toBool()
                && AppSettings::Tray->IgnoreFirstConnected->read().toBool()) {
                showPopup = false;
            }
        }
    }

    if (showPopup) {
        updateStateText(stateText, iconForState(state));
    } else {
        updateTooltip(stateText);
    }

    m_state = state;
    m_isFirstChange = false;
}

QString TrayIcon::textForState(const NordVpnInfo &state) const
{
    if (state.status() == NordVpnInfo::Status::Unknown && !AppSettings::Monitor->Active->read().toBool()) {
        return tr("<b>NordVPN status is unknown</b><br>Enable monitoring for automatic status updates.");
    }

    return state.toString();
}

void TrayIcon::deployDefaults() const
{
    static const QString rscPath(":/icn/resources/tray/%1");
    static const QStringList names {
        QLatin1String("unknown"),
        QLatin1String("disconnected"),
        QLatin1String("connecting"),
        QLatin1String("connected"),
    };
    static const QStringList suffixes { QLatin1String(), QLatin1String("_sub") };

    for (const auto &part : names) {
        for (const auto &suffix : suffixes) {
            const auto &fsFile = GroupTray::iconPath(QString("%1%2.png").arg(part, suffix));
            const QFileInfo info(fsFile);
            if (!info.exists()) {
                const auto &resourceFile = rscPath.arg(info.fileName());
                if (!QFile::copy(resourceFile, fsFile)) {
                    WRN << "Failed to deploy default icon:" << resourceFile << "->" << fsFile;
                }
            }
        }
    }

    static const QStringList kdeNames {
        QLatin1String("unknown"),
        QLatin1String("disconnected"),
        QLatin1String("connected"),
    };

    static const QLatin1String kdeSubdir("kde/%1.png");
    for (const auto &part : kdeNames) {
        const auto &fsFile = GroupTray::iconPath(kdeSubdir.arg(part));
        const QFileInfo info(fsFile);
        if (!info.exists()) {
            const auto &resourceFile = rscPath.arg(kdeSubdir.arg(part));
            if (!QFile::copy(resourceFile, fsFile)) {
                WRN << "Failed to deploy default icon:" << resourceFile << "->" << fsFile;
            }
        }
    }
}

void TrayIcon::updateTooltip(const QString &text)
{
    const QString &sanitized = QTextDocumentFragment::fromHtml(text).toPlainText();
    setToolTip(sanitized);
}

void TrayIcon::showUpdateNotification(const QString &version, const QUrl &repoUrl)
{
    disconnect(this, &QSystemTrayIcon::messageClicked, nullptr, nullptr);
    connect(
            this, &QSystemTrayIcon::messageClicked, this, [repoUrl]() { QDesktopServices::openUrl(repoUrl); },
            Qt::SingleShotConnection);

    const QString text = tr("New version %1 is available\n%2").arg(version, repoUrl.toString());
    const QString &tooltip = QTextDocumentFragment::fromHtml(text).toPlainText();
    setToolTip(tooltip);
    showMessage(qApp->applicationDisplayName(), tooltip, QSystemTrayIcon::Information,
                static_cast<int>(m_duration.count()));
}

void TrayIcon::updateStateText(const QString &message, QSystemTrayIcon::MessageIcon messageType)
{
    const QString &tooltip = QTextDocumentFragment::fromHtml(message).toPlainText();
    setToolTip(tooltip);

    if (messageType != QSystemTrayIcon::NoIcon) {
        const bool forcePlainText = AppSettings::Tray->MessagePlainText->read().toBool();
        const auto &sanitized = forcePlainText ? tooltip : message;
        showMessage(qApp->applicationDisplayName(), sanitized, messageType, static_cast<int>(m_duration.count()));
    }
}

void TrayIcon::updateStateText(const QString &message, const QIcon &icon)
{
    const QString &tooltip = QTextDocumentFragment::fromHtml(message).toPlainText();
    setToolTip(tooltip);

    if (!icon.isNull()) {
        const bool forcePlainText = AppSettings::Tray->MessagePlainText->read().toBool();
        const auto &sanitized = forcePlainText ? tooltip : message;
        showMessage(qApp->applicationDisplayName(), sanitized, icon, static_cast<int>(m_duration.count()));
    }
}
