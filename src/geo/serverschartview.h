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

#pragma once

#include "app/nordvpninfo.h"
#include "geo/mapwidget.h"
#include "geo/placeinfo.h"
#include "geo/serverlocationresolver.h"

#include <QWidget>

class NordVpnWraper;
class MapServersModel;
class ServersFilterModel;
class QLineEdit;
class QTreeView;
class QToolButton;
class QProgressBar;
class QTimer;

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

    void onMarkerDoubleclicked(const PlaceInfo &place);
    void saveServerLocationsCache();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    static QPointer<ServersChartView> m_instance;
    explicit ServersChartView(NordVpnWraper *nordVpnWraper, QWidget *parent = {});

    QLineEdit *m_searchBox { nullptr };
    QTreeView *m_treeView { nullptr };
    QToolButton *m_buttonReload { nullptr };
    QProgressBar *m_progressBar { nullptr };
    MapWidget *m_chartWidget { nullptr };

    NordVpnWraper *m_nordVpnWraper { nullptr };
    ServerLocationResolver *m_listManager { nullptr };
    MapServersModel *m_serversModel { nullptr };
    ServersFilterModel *m_serversFilterModel { nullptr };
    QTimer *m_timer { nullptr };

    void initUi();
    void initConenctions();
    void loadSettings();

    void requestServersList();

    void requestConnection(const PlaceInfo &place);

    void handleLocationReadingPorgress(int current, int total);
};
