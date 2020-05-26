/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QVector>

class NordVpnWraper;
class ServersListManager : public QObject
{
    Q_OBJECT
public:
    using Servers = QVector<QString>;
    using Group = QPair<QString, ServersListManager::Servers>;
    using Groups = QVector<ServersListManager::Group>;

    explicit ServersListManager(NordVpnWraper *nordVpn, QObject *parent = nullptr);

    bool reload();

signals:
    void ready(const Groups &groups, const Groups &countries) const;

private slots:
    void onFinished();

private:
    NordVpnWraper *m_nordVpn;

    Groups m_groups;
    Groups m_countries;

    bool m_gotGroups, m_gotCountries;

    QFutureWatcher<void> m_futureWatcher;

    Servers queryGroups() const;
    Servers queryCountries() const;
    Servers queryCities(const QString &country) const;

    void run();
    void runSeparated();

    static Servers stringToServers(const QString &in);
    Servers queryList(const QStringList &args) const;
};

Q_DECLARE_METATYPE(ServersListManager::Servers);
Q_DECLARE_METATYPE(ServersListManager::Group);
