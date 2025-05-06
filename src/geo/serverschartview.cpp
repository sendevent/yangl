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

#include "serverschartview.h"

#include "app/common.h"
#include "app/nordvpnwraper.h"
#include "app/statechecker.h"
#include "geo/flatplaceproxymodel.h"
#include "geo/mapserversmodel.h"
#include "serversfiltermodel.h"
#include "settings/appsettings.h"

#include <QBoxLayout>
#include <QCompleter>
#include <QHideEvent>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>

/*static*/ QPointer<ServersChartView> ServersChartView::m_instance = {};

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServerLocationResolver(m_nordVpnWraper, this))
    , m_serversModel(new MapServersModel(this))
    , m_serversFilterModel(new ServersFilterModel(this))
    , m_timer(new QTimer(this))
{
    m_serversFilterModel->setSourceModel(m_serversModel);

    initUi();
    initConenctions();
    loadSettings();

    requestServersList();
}

ServersChartView::~ServersChartView()
{
    m_timer->stop();
}

void ServersChartView::initUi()
{
    setWindowTitle(utils::composeTitle("Yet Another NordVPN GUI for Linux"));

    QWidget *leftView = new QWidget(this);
    QVBoxLayout *leftVBox = new QVBoxLayout(leftView);
    leftVBox->setContentsMargins(0, 0, 0, 0);

    auto serversProxyModle = new FlatPlaceProxyModel(this);
    serversProxyModle->setSourceModel(m_serversModel);

    m_searchBox->setToolTip(tr("Filter by country/city"));
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setCompleter([this, serversProxyModle]() {
        auto *completer = new QCompleter(serversProxyModle, this);
        completer->setCompletionColumn(0);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        completer->setCompletionRole(Qt::DisplayRole);
        return completer;
    }());

    m_treeView = new QTreeView(leftView);
    m_treeView->setModel(m_serversFilterModel);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setHeaderHidden(true);
    m_treeView->setSortingEnabled(true);

    QHBoxLayout *hBox = new QHBoxLayout;
    hBox->setAlignment(Qt::AlignCenter);
    m_buttonReload = new QToolButton(leftView);
    m_buttonReload->setText("Reload");
    m_buttonReload->setToolTip(tr("Update available servers list"));
    hBox->addWidget(m_buttonReload);

    m_progressBar = new QProgressBar(leftView);
    m_progressBar->setRange(0, 0);
    hBox->addWidget(m_progressBar);
    m_progressBar->hide();

    m_chartWidget = new MapWidget(AppSettings::Map->MapPlugin->read().toString(),
                                  AppSettings::Map->MapType->read().toInt(), serversProxyModle, this);
    m_chartWidget->init();

    leftVBox->addWidget(m_searchBox);
    leftVBox->addWidget(m_treeView);
    leftVBox->addItem(hBox);
    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(leftView);
    splitter->addWidget(m_chartWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(splitter);
}

void ServersChartView::initConenctions()
{
    connect(m_listManager, &ServerLocationResolver::serverLocationResolved, this, &ServersChartView::onGotLocation);

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current, const QModelIndex &) { onCurrentTreeItemChanged(current); });
    connect(m_treeView, &QTreeView::pressed, this, &ServersChartView::onCurrentTreeItemChanged);
    connect(m_treeView, &QTreeView::doubleClicked, this, &ServersChartView::onTreeItemDoubleclicked);
    connect(m_searchBox, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_serversFilterModel->setFilterRegularExpression(text); });
    connect(m_chartWidget, &MapWidget::markerDoubleclicked, this, &ServersChartView::onMarkerDoubleclicked);
    connect(m_buttonReload, &QToolButton::clicked, this, &ServersChartView::onReloadRequested);

    m_timer->setInterval(3 * utils::oneSecondMs());
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ServersChartView::saveServerLocationsCache);
}

void ServersChartView::loadSettings()
{
    m_searchBox->setText(AppSettings::Map->Filter->read().toString());

    const auto [coord, parsed] = utils::parseCoordinates(AppSettings::Map->CenterLat->read().toString(),
                                                         AppSettings::Map->CenterLon->read().toString());
    if (parsed) {
        m_chartWidget->centerOn(coord);
    }

    bool ok(false);
    const double scale = AppSettings::Map->Scale->read().toDouble(&ok);
    if (ok) {
        m_chartWidget->setScale(scale);
    }

    restoreGeometry(AppSettings::Map->Geometry->read().toByteArray());
    setVisible(AppSettings::Map->Visible->read().toBool());
}

void ServersChartView::saveSettings()
{
    AppSettings::Map->Geometry->write(saveGeometry());
    AppSettings::Map->Filter->write(m_searchBox->text());

    const QGeoCoordinate coord = m_chartWidget->center();
    AppSettings::Map->CenterLat->write(coord.latitude());
    AppSettings::Map->CenterLon->write(coord.longitude());

    AppSettings::Map->Scale->write(m_chartWidget->scale());
}

void ServersChartView::hideEvent(QHideEvent *event)
{
    saveSettings();
    QWidget::hideEvent(event);
}

void ServersChartView::requestServersList()
{
    m_listManager->refresh();
}

void ServersChartView::onReloadRequested()
{
    handleLocationReadingPorgress(1, 150);
    requestServersList();
}

void ServersChartView::onGotLocation(const PlaceInfo &place, int current, int total)
{
    LOG << place.country << place.town << place.location << current << total;

    handleLocationReadingPorgress(current, total);

    if (!place.ok) {
        WRN << place.country << place.town << place.message;
        return;
    }

    PlaceInfo group(place);
    group.country = utils::nvpnToGeo(place.country);
    group.town = utils::nvpnToGeo(place.town);
    m_serversModel->addMarker(group);
}

void ServersChartView::onCurrentTreeItemChanged(const QModelIndex &current)
{
    const auto &place = current.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();
    if (!place.isGroup() && place.location.isValid()) {
        m_chartWidget->centerOn(place.location);
    }
}

void ServersChartView::onTreeItemDoubleclicked(const QModelIndex &current)
{
    const auto &place = current.data(MapServersModel::Roles::PlaceInfoRole).value<PlaceInfo>();
    requestConnection(place);
}

void ServersChartView::onMarkerDoubleclicked(const PlaceInfo &place)
{
    requestConnection(place);
}

void ServersChartView::requestConnection(const PlaceInfo &place)
{
    if (!place.ok) {
        WRN << "invalid place, ignored:" << place.country << place.town << place.message;
        return;
    }

    const auto &country = utils::geoToNvpn(place.country);
    const auto &city = utils::geoToNvpn(place.town);

    if (place.isGroup() && city.isEmpty())
        return;

    m_nordVpnWraper->connectTo(country, city);
}

void ServersChartView::onStateChanged(const NordVpnInfo &info)
{
    m_chartWidget->setActiveConnection({ info.country(), info.city() });
}

/*static*/ void ServersChartView::makeVisible(NordVpnWraper *nordVpnWraper)
{
    if (!m_instance) {
        m_instance = new ServersChartView(nordVpnWraper);
        if (auto stateChecker = nordVpnWraper->stateChecker()) {
            connect(stateChecker, &StateChecker::stateChanged, m_instance, &ServersChartView::onStateChanged);
            m_instance->onStateChanged(stateChecker->state());
        }
    }

    if (m_instance) {
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
        m_instance->show();
        m_instance->activateWindow();
        m_instance->raise();
    }
}

void ServersChartView::handleLocationReadingPorgress(int current, int total)
{
    const bool finished = current == total;
    if (finished) {
        m_timer->stop();
        m_timer->start();
    }
}

void ServersChartView::saveServerLocationsCache()
{
    m_progressBar->setHidden(true);
    m_buttonReload->setVisible(true);
    m_listManager->saveCache();
}
