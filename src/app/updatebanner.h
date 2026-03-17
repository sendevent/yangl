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

#include <QFrame>
#include <QUrl>

class QLabel;
class NordVpnWrapper;

class UpdateBanner : public QFrame
{
    Q_OBJECT
public:
    explicit UpdateBanner(bool dismissible, QWidget *parent = {});

    static UpdateBanner *create(bool dismissible, NordVpnWrapper *wrapper, QWidget *parent);

    void setUpdate(const QString &version, const QUrl &url);
    void setNordVpnUpdate(const QUrl &url);

private:
    void refreshText();

    QLabel *m_label { nullptr };
    QString m_appVersion;
    QUrl m_appUpdateUrl;
    bool m_hasNordVpnUpdate { false };
    QUrl m_nordVpnUpdateUrl;
};
