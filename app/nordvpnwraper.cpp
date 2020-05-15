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

#include "appsettings.h"
#include "ipcbus.h"
#include "settingsdialog.h"
#include "statechecker.h"
#include "trayicon.h"

#include <QApplication>
#include <QDebug>

#define LOG qDebug() << Q_FUNC_INFO

NordVpnWraper::NordVpnWraper(QObject *parent)
    : QObject(parent)
    , m_bus(new IPCBus(AppSettings::Monitor.NVPNPath->read().toString(), this))
    , m_checker(new StateChecker(m_bus, this))
    , m_trayIcon(new TrayIcon(this))
    , m_menu(new QMenu)
{
    connect(m_checker, &StateChecker::stateChanged, m_trayIcon, &TrayIcon::setState);
    connect(qApp, &QApplication::aboutToQuit, this, &NordVpnWraper::saveSettings);

    m_trayIcon->setVisible(true);
    m_trayIcon->setContextMenu(m_menu.get());
}

void NordVpnWraper::start()
{
    initMenu();
    loadSettings();
}

void NordVpnWraper::loadSettings()
{
    const bool wasActive = m_checker->isActive();
    m_actRun->setChecked(false);

    m_checker->setInterval(AppSettings::Monitor.Interval->read().toInt() * 1000);
    m_actRun->setChecked(wasActive || AppSettings::Monitor.Active->read().toBool());
}

void NordVpnWraper::saveSettings() {}

void NordVpnWraper::initMenu()
{
    m_actSettings = m_menu->addAction(tr("Show &settings"), this, &NordVpnWraper::showSettingsEditor);
    m_actCheckState = m_menu->addAction(tr("&Check status"), this, &NordVpnWraper::performStatusCheck);
    m_actRun = m_menu->addAction(tr("&Run"));
    m_actRun->setCheckable(true);
    connect(m_actRun, &QAction::toggled, m_checker, &StateChecker::setActive);

    m_menu->addSeparator();
    m_actQuit = m_menu->addAction(tr("&Quit"), qApp, &QApplication::quit);
}

void NordVpnWraper::showSettingsEditor()
{
    Dialog *dlg = new Dialog;
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::finished, this, [this](int result) {
        if (result == QDialog::Accepted) {
            loadSettings();
        }
    });
    dlg->open();
}

void NordVpnWraper::performStatusCheck()
{
    m_checker->check();
}
