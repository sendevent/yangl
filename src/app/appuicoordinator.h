/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#include <QObject>
#include <QSystemTrayIcon>

class NordVpnWrapper;
class ActionStorage;
class StateChecker;

class AppUiCoordinator : public QObject
{
    Q_OBJECT
public:
    AppUiCoordinator(NordVpnWrapper *wrapper, ActionStorage *storage, StateChecker *checker, QObject *parent = {});

    void saveState() const;

public slots:
    void showMapView();
    void showSettingsEditor();
    void showLog();
    void showAbout();

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

signals:
    void settingsAccepted();
    void actionRequested(int nordVpnAction);

private:
    NordVpnWrapper *m_wrapper;
    ActionStorage *m_storage;
    StateChecker *m_checker;
};
