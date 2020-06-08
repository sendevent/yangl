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
   along with this program. If not, see
   <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "settingsdialog.h"

#include "action.h"
#include "actioneditor.h"
#include "actionstorage.h"
#include "appsettings.h"
#include "common.h"
#include "ui_settingsdialog.h"

#ifndef YANGL_NO_GEOCHART
#include "mapsettings.h"
#include "mapwidget.h"
#endif

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QMetaEnum>

/*static*/ QPointer<SettingsDialog> SettingsDialog::m_instance = {};

SettingsDialog::SettingsDialog(ActionStorage *actStorage, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_actStorage(actStorage)
#ifndef YANGL_NO_GEOCHART
    , m_mapSettings(new MapSettings(this))
#else
    , m_mapSettings(nullptr)
#endif
{
    ui->setupUi(this);

    setWindowTitle(tr("%1 — Settings").arg(qApp->applicationDisplayName()));

    connect(ui->checkBoxAutoActive, &QCheckBox::toggled, ui->cbIgnoreFirstConnected, &QCheckBox::setEnabled);

    ui->leNVPNPath->setText(AppSettings::Monitor.NVPNPath->read().toString());
    ui->spinBoxInterval->setValue(AppSettings::Monitor.Interval->read().toInt() / yangl::OneSecondMs);
    ui->spinBoxMsgDuration->setValue(AppSettings::Monitor.MessageDuration->read().toInt());
    ui->checkBoxAutoActive->setChecked(AppSettings::Monitor.Active->read().toBool());
    ui->cbIgnoreFirstConnected->setChecked(AppSettings::Monitor.IgnoreFirstConnected->read().toBool());
    ui->checkBoxMessagePlainText->setChecked(AppSettings::Monitor.MessagePlainText->read().toBool());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    ui->tabNordVpn->setActions(m_actStorage, Action::Flow::NordVPN);
    ui->tabCustom->setActions(m_actStorage, Action::Flow::Custom);
    ui->spinBoxLogLines->setValue(AppSettings::Monitor.LogLinesLimit->read().toInt());

#ifndef YANGL_NO_GEOCHART
    ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, m_mapSettings);
#endif

    restoreGeometry(AppSettings::Monitor.SettingsDialog->read().toByteArray());
}

SettingsDialog::~SettingsDialog()
{
    AppSettings::Monitor.SettingsDialog->write(saveGeometry());
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

#ifndef YANGL_NO_GEOCHART
    AppSettings::Map.MapType->write(m_mapSettings->selectedType());
    AppSettings::Map.MapPlugin->write(m_mapSettings->selectedPlugin());

    LOG << m_mapSettings->selectedPlugin() << m_mapSettings->selectedType();
#endif

    return settingsOk && actionsOk;
}

bool SettingsDialog::saveMonitorSettings()
{
    const QString &path = ui->leNVPNPath->text();
    if (path != AppSettings::Monitor.NVPNPath->read().toString()) {
        if (!Action::isValidAppPath(path)) {
            QMessageBox::critical(this, tr("NordVPN binary"), tr("Please, specefy valid path."));
            ui->tabWidget->setCurrentIndex(0);
            ui->leNVPNPath->setFocus();

            return false;
        }
        QMessageBox::information(this, tr("NordVPN binary"), tr("Please, restart the application to apply changes."));
    }

    AppSettings::Monitor.NVPNPath->write(path);
    AppSettings::Monitor.MessageDuration->write(ui->spinBoxMsgDuration->value());
    AppSettings::Monitor.Interval->write(ui->spinBoxInterval->value() * yangl::OneSecondMs);
    AppSettings::Monitor.Active->write(ui->checkBoxAutoActive->isChecked());
    AppSettings::Monitor.IgnoreFirstConnected->write(ui->cbIgnoreFirstConnected->isChecked());
    AppSettings::Monitor.MessagePlainText->write(ui->checkBoxMessagePlainText->isChecked());
    AppSettings::Monitor.LogLinesLimit->write(ui->spinBoxLogLines->value());

    return true;
}

bool SettingsDialog::saveActions()
{
    const bool saved = ui->tabNordVpn->save() && ui->tabCustom->save();
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
