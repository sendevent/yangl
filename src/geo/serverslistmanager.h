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

#include "geo/placeinfo.h"

#include <QFutureWatcher>
#include <QObject>
#include <QSet>

class ServersListManager : public QObject
{
    Q_OBJECT
public:
    explicit ServersListManager(QObject *parent = {});
    ~ServersListManager() override;

    bool reload(const QSet<QString> &skipCountries = {});
    QString queryVersion() const;

signals:
    void ready();
    void citiesAdded(const Places &cities);
    void discoveryProgress(int processed, int total);

private slots:
    void run();

private:
    QFutureWatcher<void> m_futureWatcher;
    QSet<QString> m_skipCountries;

    Places queryGroups() const;
    Places queryCountries() const;
    Places queryCities(const QString &country) const;

    QStringList queryList(const QStringList &args) const;

    void runSeparated();

    static QStringList stringToServers(const QString &in);

    void notifyPlacesAdded(const Places &cities);
};
