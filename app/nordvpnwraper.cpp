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

#include "actionstorage.h"
#include "appsettings.h"
#include "clibus.h"
#include "menuholder.h"
#include "settingsdialog.h"
#include "statechecker.h"
#include "trayicon.h"

#include <QApplication>
#include <QDebug>
#include <QInputDialog>
#include <QTimer>
#include <QVariant>

#define LOG qDebug() << Q_FUNC_INFO

static constexpr int OneSecondMs = 1000;
static constexpr int TimeQuantMs = 60 * OneSecondMs;

NordVpnWraper::NordVpnWraper(QObject *parent)
    : QObject(parent)
    , m_bus(new CLIBus(AppSettings::Monitor.NVPNPath->read().toString(), this))
    , m_actions(new ActionStorage(this))
    , m_checker(new StateChecker(m_bus, m_actions, this))
    , m_trayIcon(new TrayIcon(this))
    , m_menuHolder(new MenuHolder(this))
    , m_pauseTimer(new QTimer(this))
    , m_paused(0)
{
    connect(qApp, &QApplication::aboutToQuit, this, &NordVpnWraper::prepareQuit);
    connect(m_checker, &StateChecker::stateChanged, m_trayIcon, &TrayIcon::setState);
    connect(m_checker, &StateChecker::statusChanged, this, &NordVpnWraper::onStatusChanged);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &NordVpnWraper::onTrayIconActivated);
    connect(m_menuHolder, &MenuHolder::actionTriggered, this, &NordVpnWraper::onActionTriggered);
    connect(m_pauseTimer, &QTimer::timeout, this, &NordVpnWraper::onPauseTimer);

    m_trayIcon->setVisible(true);
}

void NordVpnWraper::start()
{
    const bool wasActive = m_checker->isActive();

    loadSettings();

    m_trayIcon->setContextMenu(m_menuHolder->createMenu(m_actions->load()));

    if (auto actRun = m_menuHolder->getActRun()) {
        connect(actRun, &QAction::toggled, m_checker, &StateChecker::setActive);
        actRun->setChecked(wasActive || AppSettings::Monitor.Active->read().toBool());
    }
    connect(m_menuHolder->getActShowSettings(), &QAction::triggered, this, &NordVpnWraper::showSettingsEditor);
    connect(m_menuHolder->getActQuit(), &QAction::triggered, qApp, &QApplication::quit);
}

void NordVpnWraper::loadSettings()
{
    m_checker->setInterval(AppSettings::Monitor.Interval->read().toInt() * 1000);
    m_trayIcon->setMessageDuration(AppSettings::Monitor.MessageDuration->read().toInt() * 1000);
}

void NordVpnWraper::prepareQuit()
{
    disconnect(m_trayIcon);
    disconnect(m_checker);
}

void NordVpnWraper::showSettingsEditor()
{
    Dialog *dlg = new Dialog(m_actions);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::finished, this, [this](int result) {
        if (result == QDialog::Accepted) {
            start();
        }
    });
    dlg->open();
}

void NordVpnWraper::performStatusCheck()
{
    m_checker->check();
}

void NordVpnWraper::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        performStatusCheck();
        break;
    case QSystemTrayIcon::MiddleClick:
        qApp->quit();
        break;
    default:
        break;
    }
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
            break;
        }
        default:
            m_bus->performAction(action);
            break;
        }
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
        duration = QInputDialog::getInt(nullptr, tr("%1 — pause VPN for").arg(qApp->applicationDisplayName()),
                                        tr("Minutes:"), 1, 1, 1440);
        break;
    }
    default:
        return;
    }

    LOG << "requested pause:" << duration;
    if (!duration)
        return;

    m_paused = duration * TimeQuantMs;

    if (auto disconnect = m_actions->action(KnownAction::Disconnect)) {
        LOG << "disconnecting...";
        onActionTriggered(disconnect.get());
        m_pauseTimer->start(OneSecondMs);
        LOG << "timer started for" << OneSecondMs << m_paused;
    }
}

void NordVpnWraper::onPauseTimer()
{
    LOG << 1 << m_paused;
    m_paused -= OneSecondMs;
    LOG << 2 << m_paused;
    if (m_paused <= 0) {
        m_pauseTimer->stop();
        const StateChecker::Info &currentState = m_checker->state();
        if (currentState.m_status == StateChecker::Status::Unknown
            || currentState.m_status == StateChecker::Status::Disconnected) {
            if (auto connect = m_actions->action(KnownAction::Connect)) {
                onActionTriggered(connect.get());
            }
        }
        m_paused = 0;
    }
}

void NordVpnWraper::onStatusChanged(StateChecker::Status status)
{
    bool connected = false;

    switch (status) {
    case StateChecker::Status::Connected: {
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
}
