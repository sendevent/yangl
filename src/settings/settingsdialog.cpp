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
   along with this program. If not, see
   <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "settingsdialog.h"

#include "actions/action.h"
#include "actions/actionstorage.h"
#include "app/common.h"
#include "settings/appsettings.h"
#include "settings/mapsettings.h"
#include "ui_settingsdialog.h"

#include <QApplication>
#include <QIcon>
#include <QMessageBox>

/*static*/ QPointer<SettingsDialog> SettingsDialog::m_instance = {};

SettingsDialog::SettingsDialog(ActionStorage *actStorage, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_actStorage(actStorage)
    , m_mapSettings(new MapSettings(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("%1 — Settings").arg(qApp->applicationDisplayName()));

    connect(ui->checkBoxAutoActive, &QCheckBox::toggled, ui->cbIgnoreFirstConnected, &QCheckBox::setEnabled);

    ui->leNVPNPath->setText(AppSettings::Monitor->NVPNPath->read().toString());
    ui->spinBoxInterval->setValue(AppSettings::Monitor->Interval->read().toInt() / utils::oneSecondMs());
    ui->spinBoxMsgDuration->setValue(AppSettings::Tray->MessageDuration->read().toInt());
    ui->checkBoxAutoActive->setChecked(AppSettings::Monitor->Active->read().toBool());
    ui->cbIgnoreFirstConnected->setChecked(AppSettings::Tray->IgnoreFirstConnected->read().toBool());
    ui->checkBoxMessagePlainText->setChecked(AppSettings::Tray->MessagePlainText->read().toBool());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    ui->tabActionsYangl->setActions(m_actStorage, Action::Flow::Yangl);
    ui->tabActionsNordVPN->setActions(m_actStorage, Action::Flow::NordVPN);
    ui->tabActionsUser->setActions(m_actStorage, Action::Flow::Custom);
    ui->spinBoxLogLines->setValue(AppSettings::Monitor->LogLinesLimit->read().toInt());

    ui->iconUnknownEdit->setPath(AppSettings::Tray->IcnUnknown->read().toString());
    ui->iconUnknownSubEdit->setPath(AppSettings::Tray->IcnUnknownSub->read().toString());
    ui->iconDisconnectedEdit->setPath(AppSettings::Tray->IcnDisconnected->read().toString());
    ui->iconDisconnectedSubEdit->setPath(AppSettings::Tray->IcnDisconnectedSub->read().toString());
    ui->iconConnectingEdit->setPath(AppSettings::Tray->IcnConnecting->read().toString());
    ui->iconConnectingSubEdit->setPath(AppSettings::Tray->IcnConnectingSub->read().toString());
    ui->iconConnectedEdit->setPath(AppSettings::Tray->IcnConnected->read().toString());
    ui->iconConnectedSubEdit->setPath(AppSettings::Tray->IcnConnectedSub->read().toString());

    ui->mapVerticalLayout->insertWidget(0, m_mapSettings);

    restoreGeometry(AppSettings::Monitor->SettingsDialog->read().toByteArray());
}

SettingsDialog::~SettingsDialog()
{
    AppSettings::Monitor->SettingsDialog->write(saveGeometry());
    delete ui;
}

void SettingsDialog::accept()
{
    if (saveSettings())
        QDialog::accept();
}

bool SettingsDialog::saveSettings()
{
    const bool settingsOk = saveMonitorSettings();
    if (!settingsOk)
        WRN << "failed to save settings";

    const bool actionsOk = saveActions();
    if (!actionsOk)
        WRN << "failed to save actions";

    AppSettings::Map->MapType->write(m_mapSettings->selectedType());
    AppSettings::Map->MapPlugin->write(m_mapSettings->selectedPlugin());

    LOG << m_mapSettings->selectedPlugin() << m_mapSettings->selectedType();

    AppSettings::Tray->IcnUnknown->write(ui->iconUnknownEdit->path());
    AppSettings::Tray->IcnUnknownSub->write(ui->iconUnknownSubEdit->path());
    AppSettings::Tray->IcnDisconnected->write(ui->iconDisconnectedEdit->path());
    AppSettings::Tray->IcnDisconnectedSub->write(ui->iconDisconnectedSubEdit->path());
    AppSettings::Tray->IcnConnecting->write(ui->iconConnectingEdit->path());
    AppSettings::Tray->IcnConnectingSub->write(ui->iconConnectingSubEdit->path());
    AppSettings::Tray->IcnConnected->write(ui->iconConnectedEdit->path());
    AppSettings::Tray->IcnConnectedSub->write(ui->iconConnectedSubEdit->path());

    return settingsOk && actionsOk;
}

bool SettingsDialog::saveMonitorSettings()
{
    const QString &path = ui->leNVPNPath->text();
    if (path != AppSettings::Monitor->NVPNPath->read().toString()) {
        if (!Action::isValidAppPath(path)) {
            QMessageBox::critical(this, tr("NordVPN binary"), tr("Please, specefy a valid path."));
            ui->tabWidget->setCurrentIndex(0);
            ui->leNVPNPath->setFocus();

            return false;
        }
        QMessageBox::information(this, tr("NordVPN binary"), tr("Please, restart the application to apply changes."));
    }

    AppSettings::Monitor->NVPNPath->write(path);
    AppSettings::Tray->MessageDuration->write(ui->spinBoxMsgDuration->value());
    AppSettings::Monitor->Interval->write(ui->spinBoxInterval->value() * utils::oneSecondMs());
    AppSettings::Monitor->Active->write(ui->checkBoxAutoActive->isChecked());
    AppSettings::Tray->IgnoreFirstConnected->write(ui->cbIgnoreFirstConnected->isChecked());
    AppSettings::Tray->MessagePlainText->write(ui->checkBoxMessagePlainText->isChecked());
    AppSettings::Monitor->LogLinesLimit->write(ui->spinBoxLogLines->value());

    return true;
}

bool SettingsDialog::saveActions()
{
    const bool saved = ui->tabActionsYangl->save() && ui->tabActionsNordVPN->save() && ui->tabActionsUser->save();
    if (saved)
        m_actStorage->save();
    return saved;
}

/*static*/ SettingsDialog *SettingsDialog::makeVisible(ActionStorage *actionStorage)
{
    if (!m_instance) {
        m_instance = new SettingsDialog(actionStorage);
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
        return m_instance;
    }

    m_instance->activateWindow();
    m_instance->raise();
    return {};
}
