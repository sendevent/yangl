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

#include "nordvpnwraper.h"

#include "aboutdialog.h"
#include "actions/actionresultviewer.h"
#include "actions/actionstorage.h"
#include "app/common.h"
#include "app/menuholder.h"
#include "app/statechecker.h"
#include "app/trayicon.h"
#include "cli/clicaller.h"
#include "geo/serverschartview.h"
#include "settings/appsettings.h"
#include "settings/settingsdialog.h"

#include <QApplication>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QTimer>
#include <QVariant>
#include <QtConcurrentRun>

NordVpnWraper::NordVpnWraper(QObject *parent)
    : QObject(parent)
    , m_bus(new CLICaller(this))
    , m_actions(new ActionStorage(this))
    , m_checker(new StateChecker(m_bus, AppSettings::Monitor->Interval->read().toInt()))
    , m_trayIcon(new TrayIcon(this))
    , m_menuHolder(new MenuHolder(this))
    , m_pauseTimer(new QTimer(this))
    , m_paused(0)
    , m_mapView({})
{
    connect(qApp, &QApplication::aboutToQuit, this, &NordVpnWraper::prepareQuit);
    connect(m_checker, &StateChecker::stateChanged, m_trayIcon, &TrayIcon::setState);
    connect(m_checker, &StateChecker::statusChanged, this, &NordVpnWraper::onStatusChanged);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &NordVpnWraper::onTrayIconActivated);
    connect(m_menuHolder, &MenuHolder::actionTriggered, this, &NordVpnWraper::onActionTriggered);
    connect(m_pauseTimer, &QTimer::timeout, this, &NordVpnWraper::onPauseTimer);

    initMenu();
    m_trayIcon->setVisible(true);
}

void NordVpnWraper::initMenu()
{
    QList<Action::Ptr> actions = m_actions->load();
    if (actions.isEmpty())
        actions = m_actions->load({});
    QMenu *menu = m_menuHolder->createMenu(actions);
    m_trayIcon->setContextMenu(menu);
}

CLICaller *NordVpnWraper::bus() const
{
    return m_bus;
}

ActionStorage *NordVpnWraper::storate() const
{
    return m_actions;
}

StateChecker *NordVpnWraper::stateChecker() const
{
    return m_checker;
}

void NordVpnWraper::start()
{
    const bool wasActive = m_checker->isActive();

    loadSettings();

    initMenu();

    m_checker->setCheckAction(m_actions->action(Action::NordVPN::CheckStatus));
    if (auto act = m_menuHolder->yangleAction(Action::Yangl::Activated)) {
        act->setCheckable(true);
        act->setChecked(wasActive || AppSettings::Monitor->Active->read().toBool());
        m_checker->setActive(act->isChecked());
    }

    ActionResultViewer::updateLinesLimit();

    TrayIcon::reloadIcons();
    m_trayIcon->updateIcon(m_checker->state().status());
}

void NordVpnWraper::loadSettings()
{
    m_checker->setInterval(AppSettings::Monitor->Interval->read().toInt());
    m_trayIcon->setMessageDuration(AppSettings::Tray->MessageDuration->read().toInt() * utils::oneSecondMs());

    if (AppSettings::Map->Visible->read().toBool())
        showMapView();
}

void NordVpnWraper::prepareQuit()
{
    disconnect(m_trayIcon);
    disconnect(m_checker);

    const bool visible = m_mapView ? m_mapView->isVisible() : false;
    AppSettings::Map->Visible->write(visible);
    AppSettings::sync();
}

void NordVpnWraper::performStatusCheck()
{
    m_checker->check();
}

void NordVpnWraper::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    Action::NordVPN invokeMe(Action::NordVPN::Unknown);
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        if (!m_mapView)
            showMapView();
        else {
            m_mapView->activateWindow();
            m_mapView->raise();
        }
        return;
    }
    case QSystemTrayIcon::MiddleClick: {
        switch (m_checker->state().status()) {
        case NordVpnInfo::Status::Connected: {
            invokeMe = Action::NordVPN::Disconnect;
            break;
        }
        case NordVpnInfo::Status::Disconnected: {
            invokeMe = Action::NordVPN::Connect;
            break;
        }
        case NordVpnInfo::Status::Unknown: {
            invokeMe = Action::NordVPN::CheckStatus;
            break;
        }
        default:
            return;
        }
        break;
    }
    default:
        return;
    }

    if (invokeMe == Action::NordVPN::Unknown)
        return;

    if (const Action::Ptr &action = m_actions->action(invokeMe))
        onActionTriggered(action.get());
}

void NordVpnWraper::onActionTriggered(Action *action)
{
    if (!action)
        return;

    switch (action->scope()) {
    case Action::Flow::Yangl:
        return processYangleAction(action);
    case Action::Flow::NordVPN:
        return processNordVpnAction(action);
    case Action::Flow::Custom:
        return processUserAction(action);
    }
}

/*static*/ bool NordVpnWraper::isAcceptableAction(const Action *action, Action::Flow expectedFlow,
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

void NordVpnWraper::processYangleAction(Action *action)
{
    if (!isAcceptableAction(action, Action::Flow::Yangl, Q_FUNC_INFO))
        return;

    const Action::Yangl actType = static_cast<Action::Yangl>(action->type());
    switch (actType) {
    case Action::Yangl::ShowMap:
        showMapView();
        break;
    case Action::Yangl::ShowSettings:
        showSettingsEditor();
        break;
    case Action::Yangl::ShowLog:
        showLog();
        break;
    case Action::Yangl::Activated:
        m_checker->setActive(!m_checker->isActive());
        break;
    case Action::Yangl::ShowAbout:
        showAbout();
        break;
    case Action::Yangl::Quit: {
        qApp->quit();
        break;
    }
    default:
        break;
    }
}

void NordVpnWraper::processUserAction(Action *action)
{
    if (!isAcceptableAction(action, Action::Flow::Custom, Q_FUNC_INFO))
        return;

    m_bus->performAction(action);
}

void NordVpnWraper::processNordVpnAction(Action *action)
{
    if (!isAcceptableAction(action, Action::Flow::NordVPN, Q_FUNC_INFO))
        return;

    const Action::NordVPN actType = static_cast<Action::NordVPN>(action->type());
    switch (actType) {
    case Action::NordVPN::Pause05:
    case Action::NordVPN::Pause30:
    case Action::NordVPN::Pause60:
    case Action::NordVPN::PauseCustom: {
        pause(actType);
        return;
    }
    default:
        break;
    }

    m_bus->performAction(action);
}

void NordVpnWraper::pause(Action::NordVPN action)
{
    if (m_paused)
        return;

    int duration(0);
    switch (action) {
    case Action::NordVPN::Pause05:
        duration = 5;
        break;
    case Action::NordVPN::Pause30:
        duration = 30;
        break;
    case Action::NordVPN::Pause60:
        duration = 60;
        break;
    case Action::NordVPN::PauseCustom: {
        duration = QInputDialog::getInt({}, tr("%1 — pause VPN for").arg(qApp->applicationDisplayName()),
                                        tr("Minutes:"), 1, 1, 1440);
        break;
    }
    default:
        return;
    }

    if (!duration)
        return;

    m_paused = duration * 60 * utils::oneSecondMs();

    if (auto disconnect = m_actions->action(Action::NordVPN::Disconnect)) {
        onActionTriggered(disconnect.get());
        m_pauseTimer->start(utils::oneSecondMs());
    }
}

void NordVpnWraper::onPauseTimer()
{
    m_paused -= utils::oneSecondMs();

    if (m_paused <= 0) {
        m_pauseTimer->stop();
        const NordVpnInfo &currentState = m_checker->state();
        if (currentState.status() == NordVpnInfo::Status::Unknown
            || currentState.status() == NordVpnInfo::Status::Disconnected) {
            if (auto connect = m_actions->action(Action::NordVPN::Connect)) {
                onActionTriggered(connect.get());
            }
        }
        m_paused = 0;
    }
}

void NordVpnWraper::onStatusChanged(NordVpnInfo::Status status)
{
    bool connected = false;

    switch (status) {
    case NordVpnInfo::Status::Connected: {
        connected = true;
        if (m_pauseTimer->isActive()) {
            m_pauseTimer->stop();
            m_paused = 0;
        }
        break;
    }
    default:
        break;
    }

    updateActions(connected);
}

void NordVpnWraper::updateActions(bool connected)
{
    std::function<void(QMenu *)> manageMenuActionsEnablement;
    manageMenuActionsEnablement = [connected, &manageMenuActionsEnablement, this](QMenu *menu) {
        if (!menu)
            return;

        const auto &actions = menu->actions();
        for (const auto qAction : actions) {
            if (auto subMenu = qAction->menu()) {
                manageMenuActionsEnablement(subMenu);
                continue;
            }

            if (auto action = qAction->data().value<Action *>()) {
                if (action->scope() != Action::Flow::NordVPN)
                    continue;
                switch (static_cast<Action::NordVPN>(action->type())) {
                case Action::NordVPN::Rate1:
                case Action::NordVPN::Rate2:
                case Action::NordVPN::Rate3:
                case Action::NordVPN::Rate4:
                case Action::NordVPN::Rate5:
                case Action::NordVPN::Connect: {
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
                    qAction->setEnabled(connected && !m_pauseTimer->isActive());
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

void NordVpnWraper::connectTo(const QString &country, const QString &city)
{
    LOG << country << city;

    auto future = QtConcurrent::run([country, city, this]() -> bool {
        try {
            const Action::Ptr &action = storate()->createUserAction({});
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

void NordVpnWraper::showMapView()
{
    ServersChartView::makeVisible(this);
}

void NordVpnWraper::showSettingsEditor()
{
    if (auto dlg = SettingsDialog::makeVisible(m_actions)) {
        connect(dlg, &QDialog::finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                start();
            }
        });
        dlg->open();
    }
}

void NordVpnWraper::showLog()
{
    ActionResultViewer::makeVisible();
}

void NordVpnWraper::showAbout()
{
    AboutDialog::makeVisible(nullptr);
}
