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

#pragma once

#include "serverslistmanager.h"

#include <QWidget>

namespace Ui {
class ServersChartView;
}

class NordVpnWraper;
class QStandardItemModel;
class ServersChartView : public QWidget
{
    Q_OBJECT

public:
    explicit ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent = nullptr);
    ~ServersChartView();

private slots:
    void on_buttonReload_clicked();
    void onGotServers(const ServersListManager::Groups &groups, const ServersListManager::Groups &countries);

private:
    Ui::ServersChartView *ui;

    NordVpnWraper *m_nordVpnWraper;
    ServersListManager *m_listManager;
    QStandardItemModel *m_serversModel;

    void setControlsEnabled(bool enabled);

    void setupModel(const ServersListManager::Groups &groups);
};
