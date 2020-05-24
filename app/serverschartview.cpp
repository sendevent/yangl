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

#include <QStandardItemModel>

ServersChartView::ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServersChartView)
    , m_nordVpnWraper(nordVpnWraper)
    , m_listManager(new ServersListManager(m_nordVpnWraper, this))
    , m_serversModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    connect(m_listManager, &ServersListManager::ready, this, &ServersChartView::onGotServers);

    ui->treeView->setModel(m_serversModel);
}

ServersChartView::~ServersChartView()
{
    delete ui;
}

void ServersChartView::setControlsEnabled(bool enabled)
{
    for (auto control :
         std::initializer_list<QWidget *> { ui->lineEdit, ui->treeView, ui->chartWidget /*, ui->buttonReload*/ })
        control->setEnabled(enabled);
}

void ServersChartView::on_buttonReload_clicked()
{
    if (m_listManager->reload())
        setControlsEnabled(false);
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

    for (const auto &group : groups) {
        QStandardItem *title = new QStandardItem(group.first);
        m_serversModel->insertRow(m_serversModel->rowCount(), QList<QStandardItem *>() << title);
        for (const auto &city : group.second) {
            QStandardItem *content = new QStandardItem(city);
            title->appendRow(content);
        }
    }
}
