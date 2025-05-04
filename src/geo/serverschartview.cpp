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

#include "serverschartview.h"

#include "app/common.h"
#include "app/nordvpnwraper.h"
#include "geo/coordinatesresolver.h"
#include "geo/mapserversmodel.h"
#include "serversfiltermodel.h"
#include "settings/appsettings.h"

#include <QBoxLayout>
#include <QGeoCoordinate>
#include <QHideEvent>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QSplitter>
#include <QToolButton>
#include <QTreeView>
#include <qnamespace.h>

/*static*/ QPointer<ServersChartView> ServersChartView::m_instance = {};

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServerLocationResolver(m_nordVpnWraper, this))
    , m_serversModel(new MapServersModel(this))
    , m_serversFilterModel(new ServersFilterModel(this))
{
    m_serversFilterModel->setSourceModel(m_serversModel);

    initUi();
    initConenctions();
    loadSettings();

    requestServersList();
}

ServersChartView::~ServersChartView() { }

void ServersChartView::initUi()
{
    setWindowTitle(tr("Yet Another NordVPN GUI for Linux"));

    QWidget *leftView = new QWidget(this);
    QVBoxLayout *leftVBox = new QVBoxLayout(leftView);
    leftVBox->setContentsMargins(0, 0, 0, 0);
    m_lineEdit = new QLineEdit(leftView);
    m_lineEdit->setPlaceholderText(QStringLiteral("F"));
    m_lineEdit->setToolTip(tr("Filter by country/city"));
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

    m_chartWidget = new MapWidget(AppSettings::Map->MapPlugin->read().toString(),
                                  AppSettings::Map->MapType->read().toInt(), m_serversModel, this);
    m_chartWidget->init();

    leftVBox->addWidget(m_lineEdit);
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
    connect(m_lineEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_serversFilterModel->setFilterRegularExpression(text); });
    connect(m_chartWidget, &MapWidget::markerDoubleclicked, this, &ServersChartView::onMarkerDoubleclicked);
    connect(m_buttonReload, &QToolButton::clicked, this, &ServersChartView::onReloadRequested);
}

void ServersChartView::loadSettings()
{
    m_lineEdit->setText(AppSettings::Map->Filter->read().toString());

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
    AppSettings::Map->Filter->write(m_lineEdit->text());

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
    if (m_listManager->refresh()) {
        // setControlsEnabled(false);
    }
}

void ServersChartView::setControlsEnabled(bool enabled)
{
    for (auto control : std::initializer_list<QWidget *> { m_lineEdit, m_treeView, /*m_chartWidget , buttonReload*/ }) {
        control->setEnabled(enabled);
    }
}

void ServersChartView::onReloadRequested()
{
    requestServersList();
}

void ServersChartView::onGotLocation(const PlaceInfo &place)
{
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
    if (place.location.isValid()) {
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
    if (place.ok) {
        WRN << "invalid place, ignored:" << place.country << place.town << place.message;
        return;
    }

    const auto &country = utils::geoToNvpn(place.country);
    const auto &city = utils::geoToNvpn(place.town);

    if (country == "group" && city.isEmpty())
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
