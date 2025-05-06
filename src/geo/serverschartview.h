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

#pragma once

#include "app/nordvpninfo.h"
#include "geo/coordinatesresolver.h"
#include "geo/serverlocationresolver.h"
#include "mapwidget.h"

#include <QWidget>

class NordVpnWraper;
class MapServersModel;
class ServersFilterModel;
class QLineEdit;
class QTreeView;
class QToolButton;
class QProgressBar;

class ServersChartView : public QWidget
{
    Q_OBJECT

public:
    ~ServersChartView();
    static void makeVisible(NordVpnWraper *nordVpnWraper);

    void saveSettings();

public slots:
    void onStateChanged(const NordVpnInfo &info);

private slots:
    void onReloadRequested();
    void onGotLocation(const PlaceInfo &place, int current, int total);
    void onCurrentTreeItemChanged(const QModelIndex &current);
    void onTreeItemDoubleclicked(const QModelIndex &current);

    void onMarkerDoubleclicked(const PlaceInfo &addr);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    static QPointer<ServersChartView> m_instance;
    explicit ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent = {});

    QLineEdit *m_lineEdit;
    QTreeView *m_treeView;
    QToolButton *m_buttonReload;
    QProgressBar *m_progressBar;
    MapWidget *m_chartWidget;

    NordVpnWraper *m_nordVpnWraper;
    ServerLocationResolver *m_listManager;
    MapServersModel *m_serversModel;
    ServersFilterModel *m_serversFilterModel;

    void initUi();
    void initConenctions();
    void loadSettings();

    void requestServersList();

    void requestConnection(const PlaceInfo &place);

    void handleLocationReadingPorgress(int current, int total);
};
