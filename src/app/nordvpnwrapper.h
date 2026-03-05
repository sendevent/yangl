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

#include "actions/action.h"
#include "app/nordvpninfo.h"

#include <QObject>
#include <QUrl>

class AppUiCoordinator;
class CLICaller;
class ActionStorage;
class StateChecker;
class MenuHolder;
class PauseController;
class TrayIcon;
class UpdateChecker;

class NordVpnWrapper : public QObject
{
    Q_OBJECT
public:
    static NordVpnWrapper *instance();
    static void init();

    CLICaller *bus() const;
    ActionStorage *storage() const;
    StateChecker *stateChecker() const;

    void connectTo(const QString &country, const QString &city);

    static void registerAction(Action *act);

    QString pendingUpdateVersion() const { return m_updateVersion; }
    QUrl pendingUpdateUrl() const { return m_updateUrl; }

signals:
    void updateDetected(const QString &version, const QUrl &url);

private slots:
    void prepareQuit();

    void onActionTriggered(Action *action);
    void onStatusChanged(NordVpnInfo::Status status);

    void notifyError(const QString &errorMessage);

private:
    explicit NordVpnWrapper(QObject *parent = {});

    CLICaller *m_bus;
    ActionStorage *m_actions;
    StateChecker *m_checker;
    TrayIcon *m_trayIcon;
    MenuHolder *m_menuHolder;
    PauseController *m_pauseCtrl;
    AppUiCoordinator *m_uiCoordinator;
    void loadSettings();

    void updateActions(bool connected);

    void initMenu();
    void prependUpdateAction();
    void syncToggleSettings();

    void start();

    void processYanglAction(Action *action);
    void processNordVpnAction(Action *action);
    void processUserAction(Action *action);

    static bool isAcceptableAction(const Action *action, Action::Flow expectedFlow, const QString &callerInfo);

    static NordVpnWrapper *s_instance;

    QString m_lastCountry;
    QString m_lastCity;
    QString m_updateVersion;
    QUrl m_updateUrl;

    Action::Ptr m_geoAction;
    Action::Ptr m_settingsSyncAction;
    UpdateChecker *m_updateChecker;
};
