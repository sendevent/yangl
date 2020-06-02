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

#pragma once

#include <QObject>

class NordVpnInfo
{
    Q_GADGET

public:
    enum class Status
    {
        Unknown = 0,
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };
    Q_ENUM(Status);

    NordVpnInfo();

    void clear();
    bool operator==(const NordVpnInfo &other) const;
    bool operator!=(const NordVpnInfo &other) const;
    QString toString() const;

    static NordVpnInfo fromString(const QString &text);
    static NordVpnInfo::Status textToStatus(const QString &from);
    static QString statusToText(NordVpnInfo::Status from);
    static QString parseUptime(const QString &from);

    NordVpnInfo::Status status() const;
    void setStatus(NordVpnInfo::Status status);

    QString country() const;
    QString city() const;

private:
    Status m_status;
    QString m_server;
    QString m_country;
    QString m_city;
    QString m_ip;
    QString m_technology;
    QString m_protocol;
    QString m_traffic;
    QString m_uptime;

    static int MetaIdClass;
    static int MetaIdEnum;
};

Q_DECLARE_METATYPE(NordVpnInfo::Status)
Q_DECLARE_METATYPE(NordVpnInfo)
