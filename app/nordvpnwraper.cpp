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
#include "settingsdialog.h"
#include "statechecker.h"
#include "trayicon.h"

#include <QApplication>
#include <QDebug>
#include <QVariant>

#define LOG qDebug() << Q_FUNC_INFO

NordVpnWraper::NordVpnWraper(QObject *parent)
    : QObject(parent)
    , m_bus(new CLIBus(AppSettings::Monitor.NVPNPath->read().toString(), this))
    , m_actions(new ActionStorage(this))
    , m_checker(new StateChecker(m_bus, m_actions, this))
    , m_trayIcon(new TrayIcon(this))
    , m_menuMonitor(new QMenu(tr("Monitor")))
    , m_actSettings(nullptr)
    , m_actRun(nullptr)
    , m_actSeparatorQuick(nullptr)
    , m_actSeparatorNVPN(nullptr)
    , m_menuNordVpn(new QMenu(tr("NordVPN")))
    , m_actSeparatorUser(nullptr)
    , m_menuUser(new QMenu(tr("Extra")))
    , m_actSeparatorExit(nullptr)
    , m_actQuit(nullptr)
{
    connect(m_checker, &StateChecker::stateChanged, m_trayIcon, &TrayIcon::setState);
    connect(qApp, &QApplication::aboutToQuit, this, &NordVpnWraper::prepareQuit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &NordVpnWraper::onTrayIconActivated);

    m_trayIcon->setVisible(true);
    m_trayIcon->setContextMenu(m_menuMonitor.get());
}

void NordVpnWraper::start()
{
    createMenu();
    const bool wasActive = m_checker->isActive();

    loadSettings();
    m_actions->initActions();
    populateActions();

    m_actRun->setChecked(wasActive || AppSettings::Monitor.Active->read().toBool());

    showSettingsEditor();
}

void NordVpnWraper::loadSettings()
{
    m_actRun->setChecked(false);
    m_checker->setInterval(AppSettings::Monitor.Interval->read().toInt() * 1000);
}

void NordVpnWraper::prepareQuit()
{
    disconnect(m_trayIcon);
    disconnect(m_checker);
}

void NordVpnWraper::createMenu()
{
    m_menuMonitor->clear();

    m_actSettings = m_menuMonitor->addAction(tr("Show &settings"), this, &NordVpnWraper::showSettingsEditor);
    m_actRun = m_menuMonitor->addAction(tr("&Run"));
    m_actRun->setCheckable(true);
    connect(m_actRun, &QAction::toggled, m_checker, &StateChecker::setActive);

    m_actSeparatorQuick = m_menuMonitor->addSeparator();
    m_actSeparatorNVPN = m_menuMonitor->addSeparator();
    m_menuMonitor->addMenu(m_menuNordVpn.get());
    m_actSeparatorUser = m_menuMonitor->addSeparator();
    m_menuMonitor->addMenu(m_menuUser.get());
    m_actSeparatorExit = m_menuMonitor->addSeparator();
    m_actQuit = m_menuMonitor->addAction(tr("&Quit"), qApp, &QApplication::quit);
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

void NordVpnWraper::populateActions()
{
    QVector<Action::Ptr> quickActions, customActions, nvpnActions;

    for (const auto &act : m_actions->allActions()) {
        if (!act->isAnchorable())
            continue;

        switch (act->menuPlace()) {
        case Action::MenuPlace::Own: {
            switch (act->actionScope()) {
            case Action::ActScope::User: {
                customActions.append(act);
                break;
            }
            default: {
                nvpnActions.append(act);
                break;
            }
            }
            break;
        }
        default:
            quickActions.append(act);
            break;
        }
    }

    auto makeConnection = [this](const Action::Ptr &action, QMenu *menu, QAction *before) {
        QAction *qAct = menu->addAction(action->title());
        qAct->setData(QVariant::fromValue(action.get()));
        menu->insertAction(before, qAct);
        connect(qAct, &QAction::triggered, this, &NordVpnWraper::onActionTriggered);
    };

    QAction *insertBefore = m_actSeparatorNVPN;
    for (const auto &act : quickActions)
        makeConnection(act, m_menuMonitor.get(), insertBefore);

    m_menuNordVpn->clear();
    insertBefore = m_actSeparatorUser;
    for (const auto &act : nvpnActions)
        makeConnection(act, m_menuNordVpn.get(), insertBefore);

    insertBefore = m_actSeparatorExit;
    m_menuUser->clear();
    for (const auto &act : customActions)
        makeConnection(act, m_menuUser.get(), insertBefore);

    m_menuNordVpn->setDisabled(m_menuNordVpn->actions().isEmpty());
    m_menuUser->setDisabled(m_menuUser->actions().isEmpty());
}

void NordVpnWraper::onActionTriggered()
{
    if (auto qAction = qobject_cast<QAction *>(sender()))
        if (auto action = qAction->data().value<Action *>()) {
            m_bus->performAction(action);
        }
}
