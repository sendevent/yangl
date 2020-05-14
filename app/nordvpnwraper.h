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

#include <QMenu>
#include <QObject>
#include <memory>

class IPCBus;
class StateChecker;
class TrayIcon;
class NordVpnWraper : public QObject
{
    Q_OBJECT
public:
    explicit NordVpnWraper(QObject *parent = nullptr);

    void start();

private slots:
    void saveSettings();

    void showSettingsEditor();
    void performStatusCheck();
    void toggleMonitoring();

private:
    IPCBus *m_bus;
    StateChecker *m_checker;
    TrayIcon *m_trayIcon;
    const std::unique_ptr<QMenu> m_menu;
    QAction *m_actSettings;
    QAction *m_actCheckState;
    QAction *m_actRun;
    QAction *m_actQuit;

    void loadSettings();

    void initMenu();
};
