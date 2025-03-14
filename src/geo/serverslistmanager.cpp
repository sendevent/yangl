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
#include "settings/appsettings.h"

#include <QFutureWatcher>
#include <QtConcurrent>

static const struct {
    const QString ArgGroups = QStringLiteral("groups");
    const QString ArgCountries = QStringLiteral("countries");
    const QString ArgCountry = QStringLiteral("cities");

    const QString Groups = QObject::tr("Groups");
} Consts;

ServersListManager::ServersListManager(NordVpnWraper *nordVpn, QObject *parent)
    : QObject(parent)
    , m_nordVpn(nordVpn)
    , m_gotGroups(true)
    , m_gotCountries(true)
{
    connect(&m_futureWatcher, &QFutureWatcher<void>::finished, this, &ServersListManager::onFinished);
}

bool ServersListManager::reload()
{
    if (!m_futureWatcher.isRunning()) {
        run();
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
    QString result;

    const Action::Ptr &action = m_nordVpn->storate()->createUserAction({});
    ActionResultViewer::unregisterAction(&*action);
    action->setTitle(tr("Servers list"));
    action->setForcedShow(false);
    action->setArgs(args);

    if (auto call = action->createRequest()) {
        call->run();
        if (call->success())
            result = call->result();
    }

    return stringToServers(result);
}

ServersListManager::Servers ServersListManager::queryGroups() const
{
    return queryList({ Consts.ArgGroups });
}

ServersListManager::Servers ServersListManager::queryCountries() const
{
    return queryList({ Consts.ArgCountries });
}

ServersListManager::Servers ServersListManager::queryCities(const QString &country) const
{
    return queryList({ Consts.ArgCountry, country });
}

void ServersListManager::run()
{
    m_groups.clear();
    m_countries.clear();

    m_gotGroups = false;
    m_gotCountries = false;

    QFuture<void> future = QtConcurrent::run([this]() { this->runSeparated(); });

    m_futureWatcher.setFuture(future);
}

void ServersListManager::runSeparated()
{
    m_groups = { { Consts.Groups, queryGroups() } };

    const Servers countries = queryCountries();
    for (const auto &country : countries)
        m_countries.append({ country, queryCities(country) });
}

void ServersListManager::onFinished()
{
    emit ready(m_groups, m_countries);
}
