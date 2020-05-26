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

#include "trayicon.h"

#include <QObject>

class CLICaller;
class ActionStorage;
class StateChecker;
class MenuHolder;
class QTimer;
class NordVpnWraper : public QObject
{
    Q_OBJECT
public:
    explicit NordVpnWraper(QObject *parent = nullptr);

    void start();

    CLICaller *bus() const;
    ActionStorage *storate() const;
    void connectTo(const QString &country, const QString &city);

private slots:
    void prepareQuit();

    void showMapView();
    void showSettingsEditor();
    void showLog();
    void performStatusCheck();

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void onActionTriggered(Action *action);
    void onStatusChanged(NordVpnInfo::Status status);
    void onPauseTimer();

private:
    CLICaller *m_bus;
    ActionStorage *m_actions;
    StateChecker *m_checker;
    TrayIcon *m_trayIcon;
    MenuHolder *m_menuHolder;
    QTimer *m_pauseTimer;
    int m_paused;
    bool m_settingsShown;
    QPointer<QWidget> m_mapView;
    void loadSettings();

    void pause(KnownAction action);

    void updateActions(bool connected);
};
