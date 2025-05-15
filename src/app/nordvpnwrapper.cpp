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

#include "nordvpnwrapper.h"

#include "actions/actionresultviewer.h"
#include "actions/actionstorage.h"
#include "app/appuicoordinator.h"
#include "app/common.h"
#include "app/menuholder.h"
#include "app/pausecontroller.h"
#include "app/statechecker.h"
#include "app/trayicon.h"
#include "cli/clicaller.h"
#include "settings/appsettings.h"

#include <QApplication>
#include <QFutureWatcher>
#include <QtConcurrentRun>

NordVpnWrapper::NordVpnWrapper(QObject *parent)
    : QObject(parent)
    , m_bus(new CLICaller(this))
    , m_actions(new ActionStorage(this))
    , m_checker(new StateChecker(m_bus, AppSettings::Monitor->Interval->read().toInt()))
    , m_trayIcon(new TrayIcon(this))
    , m_menuHolder(new MenuHolder(this))
    , m_pauseCtrl(new PauseController(m_actions, m_checker, this))
    , m_uiCoordinator(new AppUiCoordinator(this, m_actions, m_checker, this))
{
    connect(qApp, &QApplication::aboutToQuit, this, &NordVpnWrapper::prepareQuit);
    connect(m_pauseCtrl, &PauseController::requestAction, this, &NordVpnWrapper::onActionTriggered);
    connect(m_checker, &StateChecker::stateChanged, m_trayIcon, &TrayIcon::setState);
    connect(m_checker, &StateChecker::statusChanged, this, &NordVpnWrapper::onStatusChanged);
    connect(m_checker, &StateChecker::error, this, &NordVpnWrapper::notifyError);
    connect(m_trayIcon, &QSystemTrayIcon::activated, m_uiCoordinator, &AppUiCoordinator::onTrayIconActivated);
    connect(m_menuHolder, &MenuHolder::actionTriggered, this, &NordVpnWrapper::onActionTriggered);
    connect(m_uiCoordinator, &AppUiCoordinator::settingsAccepted, this, [this]() { start(); });
    connect(m_uiCoordinator, &AppUiCoordinator::actionRequested, this, [this](int actionType) {
        if (const Action::Ptr &action = m_actions->action(static_cast<Action::NordVPN>(actionType))) {
            onActionTriggered(action.get());
        }
    });

    m_trayIcon->setVisible(true);
}

/*static*/ NordVpnWrapper *NordVpnWrapper::instance()
{
    static NordVpnWrapper *pInstance { nullptr };
    if (!pInstance) {
        pInstance = new NordVpnWrapper();
    }
    return pInstance;
}

/*static*/ void NordVpnWrapper::init()
{
    instance()->start();
}

/*static*/ void NordVpnWrapper::registerAction(Action *act)
{
    if (act) {
        connect(act, &Action::invocationError, instance(), &NordVpnWrapper::notifyError);
    }
}

void NordVpnWrapper::initMenu()
{
    const auto &actions = m_actions->load();
    QMenu *menu = m_menuHolder->createMenu(actions);
    m_trayIcon->setContextMenu(menu);
}

CLICaller *NordVpnWrapper::bus() const
{
    return m_bus;
}

ActionStorage *NordVpnWrapper::storage() const
{
    return m_actions;
}

StateChecker *NordVpnWrapper::stateChecker() const
{
    return m_checker;
}

void NordVpnWrapper::start()
{
    const bool wasActive = m_checker->isActive();

    loadSettings();

    initMenu();

    m_checker->setCheckAction(m_actions->action(Action::NordVPN::CheckStatus));
    if (auto act = m_menuHolder->yanglAction(Action::Yangl::Activated)) {
        act->setCheckable(true);
        act->setChecked(wasActive || AppSettings::Monitor->Active->read().toBool());
        m_checker->setActive(act->isChecked());
    }

    ActionResultViewer::updateLinesLimit();

    TrayIcon::reloadIcons();
    m_trayIcon->updateIcon(m_checker->state().status());
}

void NordVpnWrapper::loadSettings()
{
    m_checker->setInterval(AppSettings::Monitor->Interval->read().toInt());
    m_trayIcon->setMessageDuration(AppSettings::Tray->MessageDuration->read().toInt() * utils::oneSecondMs());

    if (AppSettings::Map->Visible->read().toBool()) {
        m_uiCoordinator->showMapView();
    }
}

void NordVpnWrapper::prepareQuit()
{
    disconnect(m_trayIcon);
    disconnect(m_checker);

    m_uiCoordinator->saveState();
    AppSettings::sync();
}

void NordVpnWrapper::onActionTriggered(Action *action)
{
    if (!action) {
        return;
    }

    switch (action->scope()) {
    case Action::Flow::Yangl: {
        return processYanglAction(action);
    }
    case Action::Flow::NordVPN: {
        return processNordVpnAction(action);
    }
    case Action::Flow::Custom: {
        return processUserAction(action);
    }
    }
}

/*static*/ bool NordVpnWrapper::isAcceptableAction(const Action *action, Action::Flow expectedFlow,
                                                  const QString &callerInfo)
{
    if (!action) {
        WRN << "No action!" << callerInfo;
        return false;
    }

    if (action->scope() != expectedFlow) {
        static const QString wrn("Unexpected Flow: %1 (expected: %2).");
        WRN << wrn.arg(QString::number(static_cast<int>(action->scope())),
                       QString::number(static_cast<int>(expectedFlow)))
            << callerInfo;
        return false;
    }

    return true;
}

void NordVpnWrapper::processYanglAction(Action *action)
{
    if (!isAcceptableAction(action, Action::Flow::Yangl, Q_FUNC_INFO)) {
        WRN << "Unexpected Yangle action:" << action;
        return;
    }

    const Action::Yangl actType = static_cast<Action::Yangl>(action->type());
    switch (actType) {
    case Action::Yangl::ShowMap: {
        m_uiCoordinator->showMapView();
        break;
    }
    case Action::Yangl::ShowSettings: {
        m_uiCoordinator->showSettingsEditor();
        break;
    }
    case Action::Yangl::ShowLog: {
        m_uiCoordinator->showLog();
        break;
    }
    case Action::Yangl::Activated: {
        m_checker->setActive(!m_checker->isActive());
        break;
    }
    case Action::Yangl::ShowAbout: {
        m_uiCoordinator->showAbout();
        break;
    }
    case Action::Yangl::Quit: {
        {
            qApp->quit();
            break;
        }
    }
    default: {
        WRN << "Unhandled Yangl action:" << actType;
        break;
    }
    }
}

void NordVpnWrapper::processUserAction(Action *action)
{
    QString errorMessage;
    if (!isAcceptableAction(action, Action::Flow::Custom, Q_FUNC_INFO)) {
        errorMessage = tr("Received instance is not a valid User Action");
    } else {
        m_bus->runCall(action->createRequest());
    }

    if (!errorMessage.isEmpty()) {
        notifyError(errorMessage);
    }
}

void NordVpnWrapper::processNordVpnAction(Action *action)
{
    if (!isAcceptableAction(action, Action::Flow::NordVPN, Q_FUNC_INFO)) {
        notifyError(tr("Unexpected NordVpn action"));
        return;
    }

    const Action::NordVPN actType = static_cast<Action::NordVPN>(action->type());
    switch (actType) {
    case Action::NordVPN::Pause05:
    case Action::NordVPN::Pause30:
    case Action::NordVPN::Pause60:
    case Action::NordVPN::PauseCustom: {
        m_pauseCtrl->pause(actType);
        return;
    }
    default: {
        break;
    }
    }

    m_bus->runCall(action->createRequest());
}

void NordVpnWrapper::onStatusChanged(NordVpnInfo::Status status)
{
    updateActions(status == NordVpnInfo::Status::Connected);
}

void NordVpnWrapper::updateActions(bool connected)
{
    std::function<void(QMenu *)> manageMenuActionsEnablement;
    manageMenuActionsEnablement = [connected, &manageMenuActionsEnablement, this](QMenu *menu) {
        if (!menu) {
            return;
        }

        const auto &actions = menu->actions();
        for (const auto qAction : actions) {
            if (auto subMenu = qAction->menu()) {
                manageMenuActionsEnablement(subMenu);
                continue;
            }

            if (auto action = qAction->data().value<Action *>()) {
                if (action->scope() != Action::Flow::NordVPN) {
                    continue;
                }
                switch (static_cast<Action::NordVPN>(action->type())) {
                case Action::NordVPN::Rate1:
                case Action::NordVPN::Rate2:
                case Action::NordVPN::Rate3:
                case Action::NordVPN::Rate4:
                case Action::NordVPN::Rate5:
                case Action::NordVPN::Connect:
                case Action::NordVPN::LogIn: {
                    qAction->setEnabled(!connected);
                    break;
                }
                case Action::NordVPN::Disconnect: {
                    qAction->setEnabled(connected);
                    break;
                }
                case Action::NordVPN::Pause05:
                case Action::NordVPN::Pause30:
                case Action::NordVPN::Pause60:
                case Action::NordVPN::PauseCustom: {
                    qAction->setEnabled(connected && !m_pauseCtrl->isPaused());
                    break;
                }
                default:
                    break;
                }
            }
        }
    };

    if (auto rootMenu = m_trayIcon->contextMenu()) {
        manageMenuActionsEnablement(rootMenu);
    }
}

void NordVpnWrapper::connectTo(const QString &country, const QString &city)
{
    LOG << country << city;

    auto future = QtConcurrent::run([country, city, this]() -> bool {
        try {
            const Action::Ptr &action = storage()->createUserAction({});
            action->setTitle(tr("Geo Connection"));
            action->setForcedShow(false);
            action->setArgs({ "c", country == utils::groupsTitle() ? "-g" : country, city });
            if (auto call = action->createRequest()) {
                call->run();
            }
            return true; // Success
        } catch (const std::exception &e) {
            WRN << "Exception in async task:" << e.what();
            return false; // Failure
        } catch (...) {
            WRN << "Unknown error in async task!";
            return false;
        }
    });

    // Check result when available
    auto *watcher = new QFutureWatcher<bool>(this);
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, this, [future, watcher]() {
        if (!future.result()) { // If false, an error occurred
            WRN << "Async task failed!";
        } else {
            WRN << "Async task completed successfully.";
        }
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void NordVpnWrapper::notifyError(const QString &errorMessage)
{
    WRN << errorMessage;
    // shows message and updates tooltip till the next error or the actual status
    m_trayIcon->updateStateText(errorMessage, QSystemTrayIcon::Warning);
}
