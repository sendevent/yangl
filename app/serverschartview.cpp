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
#include "ui_serverschartview.h"

#include <QItemSelectionModel>
#include <QStandardItemModel>

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServersChartView)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServersListManager(m_nordVpnWraper, this))
    , m_serversModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("Yet Another NordVPN GUI for Linux"));

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    connect(m_listManager, &ServersListManager::ready, this, &ServersChartView::onGotServers);

    ui->treeView->setModel(m_serversModel);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &ServersChartView::onCurrentTreeItemChanged);

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
    m_serversModel->removeRows(0, m_serversModel->rowCount());

    auto clearGeoName = [](const QString &geoName) -> QString { return QString(geoName).replace('_', ' '); };

    for (const auto &group : groups) {
        QStandardItem *title = new QStandardItem(clearGeoName(group.first));
        m_serversModel->insertRow(m_serversModel->rowCount(), QList<QStandardItem *>() << title);
        for (const auto &city : group.second) {
            QStandardItem *content = new QStandardItem(clearGeoName(city));
            title->appendRow(content);
        }
    }
}

void ServersChartView::onCurrentTreeItemChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
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
