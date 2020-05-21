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

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QMetaEnum>

Dialog::Dialog(ActionStorage *actStorage, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_actStorage(actStorage)
{
    ui->setupUi(this);

    setWindowTitle(tr("%1 — Settings").arg(qApp->applicationDisplayName()));

    connect(ui->checkBoxAutoActive, &QCheckBox::toggled, ui->cbIgnoreFirstConnected, &QCheckBox::setEnabled);

    ui->leNVPNPath->setText(AppSettings::Monitor.NVPNPath->read().toString());
    ui->spinBoxInterval->setValue(AppSettings::Monitor.Interval->read().toInt() / yangl::OneSecondMs);
    ui->spinBoxMsgDuration->setValue(AppSettings::Monitor.MessageDuration->read().toInt());
    ui->checkBoxAutoActive->setChecked(AppSettings::Monitor.Active->read().toBool());
    ui->cbIgnoreFirstConnected->setChecked(AppSettings::Monitor.IgnoreFirstConnected->read().toBool());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Dialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &Dialog::reject);

    ui->tabNordVpn->setActions(m_actStorage, Action::Scope::Builtin);
    ui->tabCustom->setActions(m_actStorage, Action::Scope::User);

    restoreGeometry(AppSettings::Monitor.SettingsDialog->read().toByteArray());
}

Dialog::~Dialog()
{
    AppSettings::Monitor.SettingsDialog->write(saveGeometry());
    delete ui;
}

void Dialog::accept()
{
    if (saveSettings())
        QDialog::accept();
}

bool Dialog::saveSettings()
{
    return saveMonitorSettings() && saveActions();
}

bool Dialog::saveMonitorSettings()
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

    return true;
}

bool Dialog::saveActions()
{
    const bool saved = ui->tabNordVpn->save() && ui->tabCustom->save();
    if (saved)
        m_actStorage->save();
    return saved;
}
