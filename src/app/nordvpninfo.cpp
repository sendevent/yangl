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

#include "nordvpninfo.h"

#include "common.h"

#include <QMetaEnum>

NordVpnInfo::NordVpnInfo()
{
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
    if (status != m_status) {
        m_status = status;
    }
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
    if (text.isEmpty()) {
        return updatedState;
    }

    const QStringList &pairs = text.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : pairs) {
        const int sep = line.indexOf(':');
        if (sep <= 0) {
            WRN << "Unexpected format:" << line;
            continue;
        }

        const QString &name = line.left(sep).simplified().toLower();
        const QString &value = line.mid(sep + 1).simplified();

        if (name.contains(QLatin1String("status"))) {
            updatedState.m_status = textToStatus(value);
        } else if (name.contains(QLatin1String("server")) || name.contains(QLatin1String("hostname"))) {
            updatedState.m_server = value;
        } else if (name.contains(QLatin1String("country"))) {
            updatedState.m_country = value;
        } else if (name.contains(QLatin1String("city"))) {
            updatedState.m_city = value;
        } else if (name.contains(QLatin1String("ip"))) {
            updatedState.m_ip = value;
        } else if (name.contains(QLatin1String("technology"))) {
            updatedState.m_technology = value;
        } else if (name.contains(QLatin1String("protocol"))) {
            updatedState.m_protocol = value;
        } else if (name.contains(QLatin1String("transfer"))) {
            updatedState.m_traffic = value;
            updatedState.m_traffic.replace(QStringLiteral("received"), QStringLiteral("↓"));
            updatedState.m_traffic.replace(QStringLiteral("sent"), QStringLiteral("↑"));
        } else if (name.contains(QLatin1String("uptime"))) {
            updatedState.m_uptime = parseUptime(value.simplified());
        }
    }

    return updatedState;
}

/*static*/ NordVpnInfo::Status NordVpnInfo::textToStatus(const QString &from)
{
    if (from.isEmpty()) {
        return NordVpnInfo::Status::Unknown;
    }

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
    const auto &parsed = tryParseUptime(from);
    if (!parsed) {
        auto err = parsed.error();
        static const QMetaEnum &me = QMetaEnum::fromType<NordVpnInfo::UptimeParseError>();
        WRN << me.key(static_cast<int>(err));
        return {};
    }
    return parsed.value();
}

/*static*/ NordVpnInfo::UptimeResult NordVpnInfo::tryParseUptime(const QString &from)
{
    if (from.isEmpty()) {
        return std::unexpected(UptimeParseError::EmptyInput);
    }

    QString result;
    bool hasParsedTokens { false };

    auto add = [&result](int value, int width = 2) {
        if (!result.isEmpty()) {
            result.append(QChar(':'));
        }
        result += QString("%1").arg(value, width, 10, QChar('0'));
    };

    const QStringList &parts = from.split(QChar(' '), Qt::SkipEmptyParts);
    for (int i = 0; i + 1 < parts.size(); i += 2) {
        bool converted(false);
        const int value = parts.at(i).toInt(&converted);
        if (!converted) {
            continue;
        }

        const QString &units = parts.at(i + 1);

        if (units.startsWith(QStringLiteral("day"))) {
            add(value, 3);
        } else {
            add(value, 2);
        }
        hasParsedTokens = true;
    }

    if (!hasParsedTokens) {
        return std::unexpected(UptimeParseError::InvalidToken);
    }

    if (result.count(QChar(':')) <= 2) {
        result.prepend(QStringLiteral("00:"));
    }

    return result;
}

QString NordVpnInfo::toString() const
{
    QString text;
    text.append(QObject::tr("<b>%1</b>").arg(statusToText(m_status)));

    if (m_status != NordVpnInfo::Status::Connected && m_status != NordVpnInfo::Status::Connecting) {
        return text;
    }

    auto add = [&text](const QString &str, const QString &delim = QStringLiteral("<br>")) {
        if (!str.isEmpty()) {
            if (!text.isEmpty()) {
                text.append(delim);
            }
            text.append(str);
        }
        return text;
    };

    if (!m_uptime.isEmpty()) {
        text = add(m_uptime, QStringLiteral(" "));
    }

    text = add(m_server);
    text = add(m_city, QStringLiteral(" — "));
    text = add(m_country, QStringLiteral(", "));
    {
        QString ipLine = m_ip;
        if (!m_technology.isEmpty() || !m_protocol.isEmpty()) {
            const QString techProto = m_protocol.isEmpty() ? m_technology
                    : m_technology.isEmpty()               ? m_protocol
                                                           : QStringLiteral("%1, %2").arg(m_technology, m_protocol);
            if (!ipLine.isEmpty()) {
                ipLine += QStringLiteral(" — ") + techProto;
            } else {
                ipLine = techProto;
            }
        }
        text = add(ipLine);
    }
    text = add(m_traffic);

    return text;
}

void NordVpnInfo::tickUptime()
{
    if (m_uptime.isEmpty()) {
        return;
    }

    const QStringList parts = m_uptime.split(QChar(':'));
    const int n = parts.size();
    if (n < 2) {
        return;
    }

    // Convert formatted uptime to total seconds.
    // Formats produced by parseUptime:
    //   "00:SS"        — 2 parts (seconds only)
    //   "00:MM:SS"     — 3 parts (minutes+seconds, leading "00" is a sentinel)
    //   "00:HH:MM:SS"  — 4 parts (hours+minutes+seconds, leading "00" is zero days)
    //   "DDD:HH:MM:SS" — 4 parts (days+hours+minutes+seconds)
    int totalSecs = 0;
    if (n == 2) {
        totalSecs = parts[1].toInt();
    } else if (n == 3) {
        totalSecs = parts[1].toInt() * 60 + parts[2].toInt();
    } else {
        totalSecs = parts[0].toInt() * 86400 + parts[1].toInt() * 3600 + parts[2].toInt() * 60 + parts[3].toInt();
    }

    ++totalSecs;

    const int ddd = totalSecs / 86400;
    const int hh = (totalSecs % 86400) / 3600;
    const int mm = (totalSecs % 3600) / 60;
    const int ss = totalSecs % 60;

    auto p = [](int v, int w) { return QString("%1").arg(v, w, 10, QChar('0')); };

    if (n >= 4 || ddd > 0) {
        const int daysWidth = ddd > 0 ? 3 : parts[0].size();
        m_uptime = p(ddd, daysWidth) + ':' + p(hh, 2) + ':' + p(mm, 2) + ':' + p(ss, 2);
    } else if (hh > 0) {
        // Promote from 3-part to 4-part as hours rolled over
        m_uptime = p(0, 3) + ':' + p(hh, 2) + ':' + p(mm, 2) + ':' + p(ss, 2);
    } else if (n == 3) {
        m_uptime = p(0, 2) + ':' + p(mm, 2) + ':' + p(ss, 2);
    } else if (mm > 0) {
        // Promote from 2-part to 3-part as minutes rolled over
        m_uptime = p(0, 2) + ':' + p(mm, 2) + ':' + p(ss, 2);
    } else {
        m_uptime = p(0, 2) + ':' + p(ss, 2);
    }
}

QString NordVpnInfo::server() const
{
    return m_server;
}

QString NordVpnInfo::country() const
{
    return m_country;
}

QString NordVpnInfo::city() const
{
    return m_city;
}
