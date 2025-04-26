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

#include "serverslistmanager.h"

#include "actions/actionresultviewer.h"
#include "actions/actionstorage.h"
#include "app/common.h"
#include "app/nordvpnwraper.h"
#include "cli/clicall.h"
#include "cli/clicaller.h"

#include <QFutureSynchronizer>
#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>
#include <qnamespace.h>

struct Consts {
    static constexpr QLatin1String ArgGroups = QLatin1String("groups");
    static constexpr QLatin1String ArgCountries = QLatin1String("countries");
    static constexpr QLatin1String ArgCountry = QLatin1String("cities");

    static constexpr QLatin1String Groups = QLatin1String("Groups");
};

ServersListManager::ServersListManager(NordVpnWraper *nordVpn, QObject *parent)
    : QObject(parent)
    , m_nordVpn(nordVpn)
{
    connect(&m_futureWatcher, &QFutureWatcher<void>::finished, this, &ServersListManager::ready);
}

bool ServersListManager::reload()
{
    if (!m_futureWatcher.isRunning()) {
        QTimer::singleShot(0, this, &ServersListManager::run);
        return true;
    }

    return false;
}

/*static*/ ServersListManager::Servers ServersListManager::stringToServers(const QString &in)
{
    return in.split('\n', Qt::SkipEmptyParts).toVector();
}

ServersListManager::Servers ServersListManager::queryList(const QStringList &args) const
{
    const Action::Ptr &action = m_nordVpn->storate()->createUserAction({});
    ActionResultViewer::unregisterAction(action.get());
    action->setTitle(tr("Servers list"));
    action->setForcedShow(false);
    action->setArgs(args);

    if (auto call = action->createRequest()) {
        call->run();
        LOG << call->result();
        if (call->success()) {
            return stringToServers(call->result());
        }
    }

    return {};
}

ServersListManager::Servers ServersListManager::queryGroups() const
{
    return queryList({ Consts::ArgGroups });
}

ServersListManager::Servers ServersListManager::queryCountries() const
{
    return queryList({ Consts::ArgCountries });
}

ServersListManager::Servers ServersListManager::queryCities(const QString &country) const
{
    return queryList({ Consts::ArgCountry, country });
}

void ServersListManager::run()
{
    m_timeCounter.start();

    QFuture<void> future = QtConcurrent::run([this]() { this->runSeparated(); });

    m_futureWatcher.setFuture(future);
}

void ServersListManager::runSeparated()
{
    auto groups = queryGroups();
    std::sort(groups.begin(), groups.end());
    emit citiesAdded({ Consts::Groups, groups });

    const auto &countries = queryCountries();
    LOG << countries;

    for (const auto &country : countries) {
        const auto &cities = queryCities(country);
        const Group group { country, cities };
        QMetaObject::invokeMethod(this, &ServersListManager::commitCities, Qt::QueuedConnection, group);
    }
}

void ServersListManager::commitCities(const ServersListManager::Group &cities)
{
    emit citiesAdded(cities);
}
