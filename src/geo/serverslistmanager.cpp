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
#include "geo/coordinatesresolver.h"
#include "settings/settingsmanager.h"

#include <QFile>
#include <QFileInfo>
#include <QFutureSynchronizer>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTimer>
#include <QtConcurrentRun>
#include <algorithm>
#include <qjsonobject.h>
#include <qnamespace.h>
#include <utility>

struct JsonConsts {
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

/*static*/ QStringList ServersListManager::stringToServers(const QString &in)
{
    return in.split('\n', Qt::SkipEmptyParts).toVector();
}

QStringList ServersListManager::queryList(const QStringList &args) const
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

PlaceInfo createPlace(const QString &country, const QString &city)
{
    PlaceInfo result;
    result.country = country;
    result.town = city;

    result.ok = true;
    result.capital = false;
    result.group = result.country == JsonConsts::ArgGroups;

    return result;
}

Places ServersListManager::queryGroups() const
{
    const auto &names = queryList({ JsonConsts::ArgGroups });
    Places groups(names.size());
    std::transform(names.begin(), names.end(), groups.begin(),
                   [](const auto &name) { return createPlace(JsonConsts::ArgGroups, name); });

    return groups;
}

Places ServersListManager::queryCountries() const
{
    const auto &names = queryList({ JsonConsts::ArgCountries });
    Places countries(names.size());
    std::transform(names.begin(), names.end(), countries.begin(),
                   [](const auto &name) { return createPlace(name, {}); });

    return countries;
}

Places ServersListManager::queryCities(const QString &country) const
{
    const auto &names = queryList({ JsonConsts::ArgCountry, country });
    Places sities(names.size());
    std::transform(names.begin(), names.end(), sities.begin(),
                   [&country](const auto &name) { return createPlace(country, name); });

    return sities;
}

void ServersListManager::run()
{
    QFuture<void> future = QtConcurrent::run([this]() { this->runSeparated(); });

    m_futureWatcher.setFuture(future);
}

void ServersListManager::runSeparated()
{
    auto groups = queryGroups();
    std::sort(groups.begin(), groups.end(), [](const auto &l, const auto &r) { return l.town < r.town; });
    notifyPlacesAdded(groups);

    const auto &countries = queryCountries();
    for (const auto &country : countries) {
        const auto &cities = queryCities(country.country);
        notifyPlacesAdded(cities);
    }
}

void ServersListManager::notifyPlacesAdded(const Places &cities)
{
    Places prepared(cities.size());
    std::transform(cities.cbegin(), cities.cend(), prepared.begin(), [](const PlaceInfo &place) {
        PlaceInfo edited(place);
        edited.country = yangl::nvpnToGeo(place.country);
        edited.town = yangl::nvpnToGeo(place.town);
        return edited;
    });
    emit citiesAdded(prepared);
}
