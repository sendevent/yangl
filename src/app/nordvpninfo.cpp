/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

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

#include "nordvpninfo.h"

#include "common.h"

#include <QMetaEnum>

/*static*/ int NordVpnInfo::MetaIdClass = -1;
/*static*/ int NordVpnInfo::MetaIdEnum = -1;

NordVpnInfo::NordVpnInfo()
{
    if (-1 == NordVpnInfo::MetaIdClass)
        NordVpnInfo::MetaIdClass = qRegisterMetaType<NordVpnInfo>();
    if (-1 == NordVpnInfo::MetaIdEnum)
        NordVpnInfo::MetaIdEnum = qRegisterMetaType<NordVpnInfo::Status>();

    clear();
}

void NordVpnInfo::clear()
{
    m_status = Status::Unknown;
    m_server.clear();
    m_country.clear();
    m_city.clear();
    m_ip.clear();
    m_technology.clear();
    m_protocol.clear();
    m_traffic.clear();
    m_uptime.clear();
}

NordVpnInfo::Status NordVpnInfo::status() const
{
    return m_status;
}

void NordVpnInfo::setStatus(NordVpnInfo::Status status)
{
    if (status != m_status)
        m_status = status;
}

bool NordVpnInfo::operator==(const NordVpnInfo &other) const
{
    return m_status == other.m_status && m_server == other.m_server && m_country == other.m_country
            && m_city == other.m_city && m_ip == other.m_ip && m_technology == other.m_technology
            && m_protocol == other.m_protocol && m_traffic == other.m_traffic && m_uptime == other.m_uptime;
}

bool NordVpnInfo::operator!=(const NordVpnInfo &other) const
{
    return !this->operator==(other);
}

/*static*/ NordVpnInfo NordVpnInfo::fromString(const QString &text)
{
    NordVpnInfo updatedState;
    if (text.isEmpty())
        return updatedState;

    const QStringList &pairs = text.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : pairs) {
        const QStringList &pair = line.split(':', Qt::SkipEmptyParts);
        if (pair.size() != 2) {
            WRN << "Unexpected format:" << line;
            continue;
        }

        const QString &name = pair.first().simplified();
        const QString &value = pair.last().simplified();

        if (name == QStringLiteral("Status"))
            updatedState.m_status = textToStatus(value);
        else if (name == QStringLiteral("Current server"))
            updatedState.m_server = value;
        else if (name == QStringLiteral("Country"))
            updatedState.m_country = value;
        else if (name == QStringLiteral("City"))
            updatedState.m_city = value;
        else if (name == QStringLiteral("Your new IP"))
            updatedState.m_ip = value;
        else if (name == QStringLiteral("Current technology"))
            updatedState.m_technology = value;
        else if (name == QStringLiteral("Current protocol"))
            updatedState.m_protocol = value;
        else if (name == QStringLiteral("Transfer")) {
            updatedState.m_traffic = value;
            updatedState.m_traffic.replace(QStringLiteral("received"), QStringLiteral("↓"));
            updatedState.m_traffic.replace(QStringLiteral("sent"), QStringLiteral("↑"));
        } else if (name == QStringLiteral("Uptime"))
            updatedState.m_uptime = parseUptime(value.simplified());
    }

    return updatedState;
}

/*static*/ NordVpnInfo::Status NordVpnInfo::textToStatus(const QString &from)
{
    if (from.isEmpty())
        return NordVpnInfo::Status::Unknown;

    const QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
    bool found(false);
    const int enumVal = me.keyToValue(qPrintable(from), &found);
    return (found ? static_cast<NordVpnInfo::Status>(enumVal) : NordVpnInfo::Status::Unknown);
}

/*static*/ QString NordVpnInfo::statusToText(NordVpnInfo::Status from)
{
    const QMetaEnum me = QMetaEnum::fromType<NordVpnInfo::Status>();
    return me.valueToKey(static_cast<int>(from));
}

/*static*/ QString NordVpnInfo::parseUptime(const QString &from)
{
    QString result;

    if (from.isEmpty())
        return result;

    auto add = [&result](int value, int width = 2) {
        if (!result.isEmpty())
            result.append(QChar(':'));
        result += QString("%1").arg(value, width, 10, QChar('0'));
    };

    const QStringList &parts = from.split(QChar(' '), Qt::SkipEmptyParts);
    for (int i = 0; i <= parts.size() - 2; ++i) {
        bool converted(false);
        const int value = parts.at(i).toInt(&converted);
        if (!converted)
            continue;

        const QString &units = parts.at(++i);

        if (units.startsWith(QStringLiteral("day")))
            add(value, 3);
        else
            add(value, 2);
    }

    if (result.count(QChar(':')) <= 2)
        result.prepend(QStringLiteral("00:"));

    return result;
}

QString NordVpnInfo::toString() const
{
    QString text;
    text.append(QObject::tr("<b>%1</b>").arg(statusToText(m_status)));

    if (m_status != NordVpnInfo::Status::Connected && m_status != NordVpnInfo::Status::Connecting)
        return text;

    auto add = [&text](const QString &str, const QString &delim = QStringLiteral("<br>")) {
        if (!text.isEmpty())
            text.append(delim);
        text.append(str);
        return text;
    };

    if (!m_uptime.isEmpty())
        text = add(m_uptime, QStringLiteral(" "));
    text = add(m_server);
    text = add(m_city, QStringLiteral(" — "));
    text = add(m_country, QStringLiteral(", "));
    text = add(m_ip);
    text = add(m_technology);
    text = add(m_protocol, QStringLiteral(", "));
    text = add(m_traffic);

    return text;
}

QString NordVpnInfo::country() const
{
    return m_country;
}

QString NordVpnInfo::city() const
{
    return m_city;
}
