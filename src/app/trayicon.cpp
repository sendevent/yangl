/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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
#include <QFileInfo>
#include <QLatin1StringView>
#include <QMetaEnum>
#include <QPainter>
#include <QPixmap>
#include <QTextDocumentFragment>

/*static*/ QMap<NordVpnInfo::Status, TrayIcon::IconInfo> TrayIcon::m_allIcons = {};
/*static*/ QMap<NordVpnInfo::Status, QIcon> TrayIcon::m_composedIcons = {};

/*static*/ void TrayIcon::reloadIcons()
{
    m_allIcons.clear();
    m_composedIcons.clear();

    QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
    for (int i = 0; i < me.keyCount(); ++i) {
        IconInfo info;
        info.m_status = static_cast<NordVpnInfo::Status>(i);
        const NordVpnInfo::Status &state = static_cast<NordVpnInfo::Status>(me.value(i));

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
            info.m_base = AppSettings::Tray->IcnConnecting->read().toString();
            info.m_sub = AppSettings::Tray->IcnConnectingSub->read().toString();
            break;
            //        case NordVpnInfo::Status::Disconnecting:
            //            info.m_base = QStringLiteral(":/icn/resources/offline.png");
            //            info.m_sub = QStringLiteral(":/icn/resources/sub_toffline.png");
            //            break;
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
    : QSystemTrayIcon(iconForStatus(NordVpnInfo::Status::Unknown), parent)
    , m_isFirstChange(true)
{
    deployDefaults();
    reloadIcons();

    updateIcon(NordVpnInfo::Status::Unknown);
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
    m_duration = durationSecs;
}

void TrayIcon::updateIcon(NordVpnInfo::Status status)
{
    const QIcon &icn = iconForStatus(status);
    if (!icn.isNull())
        setIcon(icn);
}

void TrayIcon::setState(const NordVpnInfo &state)
{
    const QString description = AppSettings::Tray->MessagePlainText->read().toBool()
            ? QTextDocumentFragment::fromHtml(state.toString()).toPlainText()
            : state.toString();

    if (m_state.status() != state.status() && !qApp->isSavingSession()) {
        updateIcon(state.status());

        bool skeepMessage(false);
        if (m_isFirstChange && state.status() == NordVpnInfo::Status::Connected)
            if (AppSettings::Monitor->Active->read().toBool()
                && AppSettings::Tray->IgnoreFirstConnected->read().toBool())
                skeepMessage = true;

        if (!skeepMessage)
            showMessage(qApp->applicationDisplayName(), description, iconForState(state), m_duration);
    }

    setToolTip(description);

    m_state = state;
    m_isFirstChange = false;
}

void TrayIcon::deployDefaults() const
{
    static const QString rscPath(":/icn/resources/tray/%1");

    for (const auto &part : {
                 QLatin1String("unknown"),
                 QLatin1String("disconnected"),
                 QLatin1String("connecting"),
                 QLatin1String("connected"),
         }) {
        for (const auto &suffix : { QLatin1String(), QLatin1String("_sub") }) {
            const auto &fsFile = GroupTray::iconPath(QString("%1%2.png").arg(part, suffix));
            const QFileInfo info(fsFile);
            if (!info.exists()) {
                const auto &resourceFile = rscPath.arg(info.fileName());
                QFile::copy(resourceFile, fsFile);
            }
        }
    }
}
