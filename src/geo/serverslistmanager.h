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

#include "geo/coordinatesresolver.h"

#include <QFutureWatcher>
#include <QList>
#include <QObject>

class NordVpnWraper;
class ServersListManager : public QObject
{
    Q_OBJECT
public:
    explicit ServersListManager(NordVpnWraper *nordVpn, QObject *parent = {});

    bool reload();

signals:
    void ready();
    void citiesAdded(const Places &cities);

private slots:
    void run();

private:
    NordVpnWraper *m_nordVpn;
    QFutureWatcher<void> m_futureWatcher;

    Places queryGroups() const;
    Places queryCountries() const;
    Places queryCities(const QString &country) const;

    QStringList queryList(const QStringList &args) const;

    void runSeparated();

    static QStringList stringToServers(const QString &in);

    void notifyPlacesAdded(const Places &cities);
};
