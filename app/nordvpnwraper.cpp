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

#include "nordvpnwraper.h"

#include "actionresultviewer.h"
#include "actionstorage.h"
#include "appsettings.h"
#include "clicaller.h"
#include "common.h"
#include "menuholder.h"

#ifndef YANGL_NO_GEOCHART
#include "serverschartview.h"
#endif // YANGL_NO_GEOCHART

#include "settingsdialog.h"
#include "statechecker.h"
#include "trayicon.h"

#include <QApplication>
#include <QInputDialog>
#include <QTimer>
#include <QVariant>
#include <QtConcurrent>

static constexpr int TimeQuantMs = 60 * yangl::OneSecondMs;

NordVpnWraper::NordVpnWraper(QObject *parent)
    : QObject(parent)
    , m_bus(new CLICaller(this))
    , m_actions(new ActionStorage(this))
    , m_checker(new StateChecker(m_bus, AppSettings::Monitor.Interval->read().toInt(), this))
    , m_trayIcon(new TrayIcon(this))
    , m_menuHolder(new MenuHolder(this))
    , m_pauseTimer(new QTimer(this))
    , m_paused(0)
    , m_settingsShown(false)
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

void NordVpnWraper::start()
{
    LOG;
    const bool wasActive = m_checker->isActive();

    loadSettings();

    initMenu();

    m_checker->setCheckAction(m_actions->action(KnownAction::CheckStatus));

#ifndef YANGL_NO_GEOCHART
    connect(m_menuHolder->getActShowMap(), &QAction::triggered, this, &NordVpnWraper::showMapView,
            Qt::UniqueConnection);
#endif

    connect(m_menuHolder->getActShowSettings(), &QAction::triggered, this, &NordVpnWraper::showSettingsEditor,
            Qt::UniqueConnection);
    connect(m_menuHolder->getActShowLog(), &QAction::triggered, this, &NordVpnWraper::showLog, Qt::UniqueConnection);
    connect(m_menuHolder->getActRun(), &QAction::toggled, m_checker, &StateChecker::setActive, Qt::UniqueConnection);

    connect(m_menuHolder->getActQuit(), &QAction::triggered, qApp, &QApplication::quit, Qt::UniqueConnection);

    m_menuHolder->getActRun()->setChecked(wasActive || AppSettings::Monitor.Active->read().toBool());

    ActionResultViewer::instance()->updateLinesLimit();
}

void NordVpnWraper::loadSettings()
{
    m_checker->setInterval(AppSettings::Monitor.Interval->read().toInt());
    m_trayIcon->setMessageDuration(AppSettings::Monitor.MessageDuration->read().toInt() * yangl::OneSecondMs);

#ifndef YANGL_NO_GEOCHART
    if (AppSettings::Map.Visible->read().toBool())
        showMapView();
#endif
}

void NordVpnWraper::prepareQuit()
{
    disconnect(m_trayIcon);
    disconnect(m_checker);

    const bool visible = m_mapView ? m_mapView->isVisible() : false;
    AppSettings::Map.Visible->write(visible);
    AppSettings::sync();
}

void NordVpnWraper::showSettingsEditor()
{
    Dialog *dlg = new Dialog(m_actions);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::finished, this, [this](int result) {
        m_settingsShown = false;
        if (result == QDialog::Accepted) {
            start();
        }
    });
    m_settingsShown = true;
    dlg->open();
}

void NordVpnWraper::showLog()
{
    ActionResultViewer::instance()->show();
    ActionResultViewer::instance()->activateWindow();
    ActionResultViewer::instance()->raise();
}

void NordVpnWraper::performStatusCheck()
{
    m_checker->check();
}

void NordVpnWraper::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    KnownAction invokeMe(KnownAction::Unknown);
    switch (reason) {
    case QSystemTrayIcon::Trigger:
#ifndef YANGL_NO_GEOCHART
        if (!m_mapView)
            showMapView();
        else {
            m_mapView->activateWindow();
            m_mapView->raise();
        }
        return;
#endif
        if (!m_settingsShown)
            showSettingsEditor();
        return;
    case QSystemTrayIcon::MiddleClick: {
        switch (m_checker->state().status()) {
        case NordVpnInfo::Status::Connected: {
            invokeMe = KnownAction::Disconnect;
            break;
        }
        case NordVpnInfo::Status::Disconnected: {
            invokeMe = KnownAction::Connect;
            break;
        }
        case NordVpnInfo::Status::Unknown: {
            invokeMe = KnownAction::CheckStatus;
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

    if (invokeMe == KnownAction::Unknown)
        return;

    if (const Action::Ptr &action = m_actions->action(invokeMe))
        onActionTriggered(action.get());
}

void NordVpnWraper::onActionTriggered(Action *action)
{
    if (action) {
        const KnownAction actType = action->type();
        switch (actType) {
        case Pause05:
        case Pause30:
        case Pause60:
        case PauseCustom: {
            pause(actType);
            return;
        }
        default:
            break;
        }
        m_bus->performAction(action);
    }
}

void NordVpnWraper::pause(KnownAction action)
{
    if (m_paused)
        return;

    int duration(0);
    switch (action) {
    case Pause05:
        duration = 5;
        break;
    case Pause30:
        duration = 30;
        break;
    case Pause60:
        duration = 60;
        break;
    case PauseCustom: {
        duration = QInputDialog::getInt({}, tr("%1 — pause VPN for").arg(qApp->applicationDisplayName()),
                                        tr("Minutes:"), 1, 1, 1440);
        break;
    }
    default:
        return;
    }

    if (!duration)
        return;

    m_paused = duration * TimeQuantMs;

    if (auto disconnect = m_actions->action(KnownAction::Disconnect)) {
        onActionTriggered(disconnect.get());
        m_pauseTimer->start(yangl::OneSecondMs);
    }
}

void NordVpnWraper::onPauseTimer()
{
    m_paused -= yangl::OneSecondMs;

    if (m_paused <= 0) {
        m_pauseTimer->stop();
        const NordVpnInfo &currentState = m_checker->state();
        if (currentState.status() == NordVpnInfo::Status::Unknown
            || currentState.status() == NordVpnInfo::Status::Disconnected) {
            if (auto connect = m_actions->action(KnownAction::Connect)) {
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

        for (auto qAction : menu->actions()) {
            if (auto subMenu = qAction->menu()) {
                manageMenuActionsEnablement(subMenu);
                continue;
            }

            if (auto action = qAction->data().value<Action *>()) {
                switch (action->type()) {
                case KnownAction::Rate1:
                case KnownAction::Rate2:
                case KnownAction::Rate3:
                case KnownAction::Rate4:
                case KnownAction::Rate5:
                case KnownAction::Connect: {
                    qAction->setEnabled(!connected);
                    break;
                }
                case KnownAction::Disconnect: {
                    qAction->setEnabled(connected);
                    break;
                }
                case KnownAction::Pause05:
                case KnownAction::Pause30:
                case KnownAction::Pause60:
                case KnownAction::PauseCustom: {
                    qAction->setEnabled(connected && !m_pauseTimer->isActive());
                    break;
                }
                default:
                    break;
                }
            }
        }
    };

    if (auto rootMenu = m_trayIcon->contextMenu())
        manageMenuActionsEnablement(rootMenu);
}

void NordVpnWraper::connectTo(const QString &country, const QString &city)
{
    LOG << country << city;

    QtConcurrent::run([country, city, this]() {
        const Action::Ptr &action = storate()->createUserAction({});
        action->setTitle(tr("Geo Connection"));
        action->setForcedShow(false);
        action->setArgs({ "c", country == "group" ? "-g" : country, city });
        if (auto call = action->createRequest()) {
            call->run();
        }
    });
}

void NordVpnWraper::showMapView()
{
#ifndef YANGL_NO_GEOCHART
    if (!m_mapView) {
        ServersChartView *chartView = new ServersChartView(this);
        connect(m_checker, &StateChecker::stateChanged, chartView, &ServersChartView::onStateChanged);
        chartView->onStateChanged(m_checker->state());
        m_mapView = chartView;
    }
#endif
    if (m_mapView) {
        m_mapView->setAttribute(Qt::WA_DeleteOnClose);
        m_mapView->show();
    }
}
