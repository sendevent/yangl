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

#include "appuicoordinator.h"

#include "aboutdialog.h"
#include "actions/action.h"
#include "actions/actionresultviewer.h"
#include "actions/actionstorage.h"
#include "app/nordvpninfo.h"
#include "app/statechecker.h"
#include "geo/serverschartview.h"
#include "settings/appsettings.h"
#include "settings/settingsdialog.h"

AppUiCoordinator::AppUiCoordinator(NordVpnWrapper *wrapper, ActionStorage *storage, StateChecker *checker,
                                   QObject *parent)
    : QObject(parent)
    , m_wrapper(wrapper)
    , m_storage(storage)
    , m_checker(checker)
{
}

void AppUiCoordinator::saveState() const
{
    const auto *mapView = ServersChartView::instance();
    const bool visible = mapView ? mapView->isVisible() : false;
    AppSettings::Map->Visible->write(visible);
}

void AppUiCoordinator::showMapView()
{
    ServersChartView::makeVisible(m_wrapper);
    if (auto *inst = ServersChartView::instance()) {
        connect(inst, &ServersChartView::requestSettings, this, &AppUiCoordinator::showSettingsEditor,
                Qt::UniqueConnection);
    }
}

void AppUiCoordinator::showSettingsEditor()
{
    if (auto dlg = SettingsDialog::makeVisible(m_storage)) {
        connect(dlg, &QDialog::finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                emit settingsAccepted();
            }
        });
        connect(dlg, &SettingsDialog::showMapRequested, this, &AppUiCoordinator::showMapView,
                Qt::UniqueConnection);
        dlg->open();
    }
}

void AppUiCoordinator::showLog()
{
    ActionResultViewer::makeVisible();
}

void AppUiCoordinator::showAbout()
{
    AboutDialog::makeVisible(nullptr);
}

void AppUiCoordinator::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        showMapView();
        return;
    }
    case QSystemTrayIcon::MiddleClick: {
        int invokeMe = static_cast<int>(Action::NordVPN::Unknown);
        switch (m_checker->state().status()) {
        case NordVpnInfo::Status::Connected:
            invokeMe = static_cast<int>(Action::NordVPN::Disconnect);
            break;
        case NordVpnInfo::Status::Disconnected:
            invokeMe = static_cast<int>(Action::NordVPN::Connect);
            break;
        case NordVpnInfo::Status::Unknown:
            invokeMe = static_cast<int>(Action::NordVPN::CheckStatus);
            break;
        default:
            return;
        }
        if (invokeMe != static_cast<int>(Action::NordVPN::Unknown)) {
            emit actionRequested(invokeMe);
        }
        return;
    }
    default:
        return;
    }
}
