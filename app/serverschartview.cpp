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

#include "serverschartview.h"

#include "common.h"
#include "nordvpnwraper.h"
#include "serversfiltermodel.h"
#include "ui_serverschartview.h"

#include <QItemSelectionModel>
#include <QStandardItemModel>

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServersChartView)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServersListManager(m_nordVpnWraper, this))
    , m_serversModel(new QStandardItemModel(this))
    , m_serversFilterModel(new ServersFilterModel(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("Yet Another NordVPN GUI for Linux"));

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    connect(m_listManager, &ServersListManager::ready, this, &ServersChartView::onGotServers);

    m_serversFilterModel->setSourceModel(m_serversModel);
    ui->treeView->setModel(m_serversFilterModel);
    ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->setHeaderHidden(true);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current, const QModelIndex &) { onCurrentTreeItemChanged(current); });
    connect(ui->treeView, &QTreeView::pressed, this, &ServersChartView::onCurrentTreeItemChanged);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &ServersChartView::onTreeItemDoubleclicked);
    connect(ui->lineEdit, &QLineEdit::textChanged, this,
            [this](const QString &text) { m_serversFilterModel->setFilterRegExp(text); });

    connect(ui->chartWidget, &MapWidget::markerDoubleclicked, this, &ServersChartView::onMarkerDoubleclicked);
    requestServersList();
}

ServersChartView::~ServersChartView()
{
    delete ui;
}

void ServersChartView::requestServersList()
{
    if (m_listManager->reload())
        setControlsEnabled(false);
}

void ServersChartView::setControlsEnabled(bool enabled)
{
    for (auto control :
         std::initializer_list<QWidget *> { ui->lineEdit, ui->treeView, ui->chartWidget /*, ui->buttonReload*/ })
        control->setEnabled(enabled);
}

void ServersChartView::on_buttonReload_clicked()
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
    ui->chartWidget->clearMarks();

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
            ui->chartWidget->addMark(group.first, {});

        for (const auto &city : group.second) {
            const QString &cityName = clearGeoName(city);
            QStandardItem *content = new QStandardItem(cityName);
            title->appendRow(content);

            if (!isGroups)
                ui->chartWidget->addMark(countryName, cityName);
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

    ui->chartWidget->centerOn(country, city);
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
