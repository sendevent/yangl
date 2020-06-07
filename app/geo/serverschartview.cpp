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

#include "appsettings.h"
#include "common.h"
#include "nordvpnwraper.h"
#include "serversfiltermodel.h"

#include <QBoxLayout>
#include <QGeoCoordinate>
#include <QHideEvent>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QSplitter>
#include <QStandardItemModel>
#include <QToolButton>
#include <QTreeView>

/*static*/ QPointer<ServersChartView> ServersChartView::m_instance = {};

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServersListManager(m_nordVpnWraper, this))
    , m_serversModel(new QStandardItemModel(this))
    , m_serversFilterModel(new ServersFilterModel(this))
{
    m_serversFilterModel->setSourceModel(m_serversModel);

    initUi();
    initConenctions();
    loadSettings();

    requestServersList();
}

ServersChartView::~ServersChartView() {}

void ServersChartView::initUi()
{
    setWindowTitle(tr("Yet Another NordVPN GUI for Linux"));

    QWidget *leftView = new QWidget(this);
    QVBoxLayout *leftVBox = new QVBoxLayout(leftView);
    m_lineEdit = new QLineEdit(leftView);
    m_lineEdit->setPlaceholderText(QStringLiteral("F"));
    m_lineEdit->setToolTip(tr("Filter by country/city"));
    m_treeView = new QTreeView(leftView);
    m_treeView->setModel(m_serversFilterModel);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setHeaderHidden(true);

    QHBoxLayout *hBox = new QHBoxLayout;
    hBox->setAlignment(Qt::AlignCenter);
    m_buttonReload = new QToolButton(leftView);
    m_buttonReload->setText("Reload");
    m_buttonReload->setToolTip(tr("Update available servers list"));
    hBox->addWidget(m_buttonReload);

    m_chartWidget = new MapWidget(AppSettings::Map.MapPlugin->read().toString(),
                                  AppSettings::Map.MapType->read().toInt(), this);
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
    connect(m_listManager, &ServersListManager::ready, this, &ServersChartView::onGotServers);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current, const QModelIndex &) { onCurrentTreeItemChanged(current); });
    connect(m_treeView, &QTreeView::pressed, this, &ServersChartView::onCurrentTreeItemChanged);
    connect(m_treeView, &QTreeView::doubleClicked, this, &ServersChartView::onTreeItemDoubleclicked);
    connect(m_lineEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_serversFilterModel->setFilterRegExp(text); });
    connect(m_chartWidget, &MapWidget::markerDoubleclicked, this, &ServersChartView::onMarkerDoubleclicked);
    connect(m_buttonReload, &QToolButton::clicked, this, &ServersChartView::onReloadRequested);
}

void ServersChartView::loadSettings()
{
    m_lineEdit->setText(AppSettings::Map.Filter->read().toString());

    const qreal lat = AppSettings::Map.CenterLat->read().toDouble();
    const qreal lon = AppSettings::Map.CenterLon->read().toDouble();
    QGeoCoordinate coord;
    coord.setLatitude(lat);
    coord.setLongitude(lon);
    m_chartWidget->centerOn(coord);
    m_chartWidget->setScale(AppSettings::Map.Scale->read().toDouble());

    restoreGeometry(AppSettings::Map.Geometry->read().toByteArray());
    setVisible(AppSettings::Map.Visible->read().toBool());
}

void ServersChartView::saveSettings()
{
    AppSettings::Map.Geometry->write(saveGeometry());
    AppSettings::Map.Filter->write(m_lineEdit->text());

    const QGeoCoordinate coord = m_chartWidget->center();
    AppSettings::Map.CenterLat->write(coord.latitude());
    AppSettings::Map.CenterLon->write(coord.longitude());

    AppSettings::Map.Scale->write(m_chartWidget->scale());
}

void ServersChartView::hideEvent(QHideEvent *event)
{
    saveSettings();
    QWidget::hideEvent(event);
}

void ServersChartView::requestServersList()
{
    if (m_listManager->reload())
        setControlsEnabled(false);
}

void ServersChartView::setControlsEnabled(bool enabled)
{
    for (auto control : std::initializer_list<QWidget *> { m_lineEdit, m_treeView, m_chartWidget /*, buttonReload*/ })
        control->setEnabled(enabled);
}

void ServersChartView::onReloadRequested()
{
    requestServersList();
}

void ServersChartView::onGotServers(const ServersListManager::Groups &groups,
                                    const ServersListManager::Groups &countries)
{
    setControlsEnabled(true);

    setupModel(groups + countries);
}

void ServersChartView::setupModel(const ServersListManager::Groups &groups)
{
    m_chartWidget->clearMarks();

    m_serversModel->removeRows(0, m_serversModel->rowCount());
    auto clearGeoName = [](const QString &geoName) -> QString { return QString(geoName).replace('_', ' '); };

    if (groups.isEmpty() || (groups.size() == 1 && groups.first().second.isEmpty())) {
        QStandardItem *title = new QStandardItem(tr("No data ☹"));
        m_serversModel->insertRow(m_serversModel->rowCount(), QList<QStandardItem *>() << title);
        title->setEnabled(false);
        return;
    }

    for (const auto &group : groups) {
        const bool isGroups = group.first == "Groups"; // TODO: read ServersListManager
        const QString &countryName = clearGeoName(group.first);
        QStandardItem *title = new QStandardItem(countryName);
        m_serversModel->insertRow(m_serversModel->rowCount(), QList<QStandardItem *>() << title);

        if (!isGroups)
            m_chartWidget->addMark(group.first, {});

        for (const auto &city : group.second) {
            const QString &cityName = clearGeoName(city);
            QStandardItem *content = new QStandardItem(cityName);
            title->appendRow(content);

            if (!isGroups)
                m_chartWidget->addMark(countryName, cityName);
        }
    }
}

void ServersChartView::onCurrentTreeItemChanged(const QModelIndex &current)
{
    QString country, city;
    if (current.parent().isValid()) {
        country = current.parent().data().toString();
        city = current.data().toString();
    } else {
        country = current.data().toString();
    }

    m_chartWidget->centerOn(country, city);
}

void ServersChartView::onTreeItemDoubleclicked(const QModelIndex &current)
{
    QString country, city;

    if (current.parent().isValid()) {
        country = current.parent().data().toString();
        city = current.data().toString();
    } else {
        country = current.data().toString();
    }
    requestConnection(country, city);
}

void ServersChartView::onMarkerDoubleclicked(const MapWidget::AddrHandler &addr)
{
    requestConnection(addr.m_country, addr.m_city);
}

void ServersChartView::requestConnection(const QString &group, const QString &server)
{
    auto clearGeoName = [](const QString &geoName) -> QString {
        if (geoName == "default")
            return {};

        if (geoName == "Groups")
            return "group";

        return QString(geoName).replace(' ', '_');
    };

    const QString &country = clearGeoName(group);
    const QString &city = clearGeoName(server);
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
#ifndef YANGL_NO_GEOCHART
        m_instance = new ServersChartView(nordVpnWraper);
        if (auto stateChecker = nordVpnWraper->stateChecker()) {
            connect(stateChecker, &StateChecker::stateChanged, m_instance, &ServersChartView::onStateChanged);
            m_instance->onStateChanged(stateChecker->state());
        }
#endif
    }

    if (m_instance) {
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
        m_instance->show();
        m_instance->activateWindow();
        m_instance->raise();
    }
}
