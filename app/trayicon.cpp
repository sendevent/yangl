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

#include "appsettings.h"
#include "common.h"

#include <QApplication>
#include <QMetaEnum>
#include <QPainter>
#include <QPixmap>
#include <QTextDocumentFragment>

struct IconInfo {
    QString m_base;
    QString m_sub;
    NordVpnInfo::Status m_status;
};

/*static*/ IconInfo infoPixmaps(const NordVpnInfo::Status forStatus)
{
    static const QMap<NordVpnInfo::Status, IconInfo> staticIcons = [] {
        QMap<NordVpnInfo::Status, IconInfo> icons;
        QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
        for (int i = 0; i < me.keyCount(); ++i) {
            IconInfo info;
            info.m_status = static_cast<NordVpnInfo::Status>(i);
            const NordVpnInfo::Status state = static_cast<NordVpnInfo::Status>(me.value(i));
            switch (state) {
            case NordVpnInfo::Status::Connected:
                info.m_base = QStringLiteral(":/icn/resources/online.png");
                //                info.m_sub = QStringLiteral(":/icn/resources/sub_online.png");
                break;
            case NordVpnInfo::Status::Disconnected:
                info.m_base = QStringLiteral(":/icn/resources/offline.png");
                //                info.m_sub = QStringLiteral(":/icn/resources/sub_offline.png");
                break;
            case NordVpnInfo::Status::Connecting:
                info.m_base = QStringLiteral(":/icn/resources/offline.png");
                info.m_sub = QStringLiteral(":/icn/resources/sub_toonline.png");
                break;
            case NordVpnInfo::Status::Disconnecting:
                info.m_base = QStringLiteral(":/icn/resources/offline.png");
                info.m_sub = QStringLiteral(":/icn/resources/sub_toffline.png");
                break;
            default:
                info.m_base = QStringLiteral(":/icn/resources/offline.png");
                info.m_sub = QStringLiteral(":/icn/resources/sub_unknown.png");
                break;
            }
            icons.insert(info.m_status, info);
        }
        return icons;
    }();

    return staticIcons.value(forStatus);
}

QIcon generateIcon(const NordVpnInfo::Status forStatus)
{
    const IconInfo &info = infoPixmaps(forStatus);
    QPixmap base(info.m_base);

    if (!info.m_sub.isEmpty()) {
        const QRect baseRect(base.rect());
        QPixmap sub = QPixmap(info.m_sub)
                              .scaled(baseRect.width() / 2, baseRect.height() / 2, Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
        const QRect subRect = sub.rect();
        QRect targetRect(subRect);
        targetRect.moveTopLeft(base.rect().center());

        QPainter p(&base);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        p.setRenderHint(QPainter::LosslessImageRendering);
#endif
        p.drawPixmap(targetRect, sub, subRect);
    }
    return QIcon(base);
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(iconForStatus(NordVpnInfo::Status::Unknown), parent)
    , m_isFirstChange(true)
{
}

/*static*/ QIcon TrayIcon::iconForState(const NordVpnInfo &state)
{
    return iconForStatus(state.status());
}

/*static*/ QIcon TrayIcon::iconForStatus(const NordVpnInfo::Status &status)
{
    static const QMap<NordVpnInfo::Status, QIcon> staticIcons = [] {
        QMap<NordVpnInfo::Status, QIcon> icons;
        QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
        for (int i = 0; i < me.keyCount(); ++i) {
            const NordVpnInfo::Status state = static_cast<NordVpnInfo::Status>(me.value(i));
            icons.insert(state, generateIcon(state));
        }
        return icons;
    }();

    return staticIcons[status];
}

void TrayIcon::setMessageDuration(int durationSecs)
{
    m_duration = durationSecs;
}

void TrayIcon::setState(const NordVpnInfo &state)
{
    const QString description = AppSettings::Tray->MessagePlainText->read().toBool()
            ? QTextDocumentFragment::fromHtml(state.toString()).toPlainText()
            : state.toString();

    if (m_state.status() != state.status() && !qApp->isSavingSession()) {
        QIcon icn = iconForStatus(state.status());
        setIcon(icn);

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
