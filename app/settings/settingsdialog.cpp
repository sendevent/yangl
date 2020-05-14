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

#include "appsettings.h"
#include "ui_settingsdialog.h"

#include <QDebug>
#include <QIcon>
#include <QMetaEnum>

#define LOG qDebug() << Q_FUNC_INFO
#define WRN qWarning() << Q_FUNC_INFO
#define NIY WRN << "Not implemented yet!"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->tabNVPN->setEnabled(false);
    ui->leNVPNPath->setText(AppSettings::Monitor.NVPNPath->read().toString());
    ui->spinBoxInterval->setValue(AppSettings::Monitor.Interval->read().toInt());
    ui->checkBoxAutoActive->setChecked(AppSettings::Monitor.Active->read().toBool());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Dialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &Dialog::reject);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::accept()
{
    if (saveSettings())
        QDialog::accept();
}

bool Dialog::saveSettings()
{
    return saveMonitorSettings() && saveNVPNSettings();
}

bool Dialog::saveMonitorSettings()
{
    NIY;
    return true;
}

bool Dialog::saveNVPNSettings()
{
    NIY;
    return true;
}
