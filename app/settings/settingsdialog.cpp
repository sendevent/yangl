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
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QMetaEnum>
#include <QPalette>
#include <QStyle>

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

void Dialog::on_btnOpen_clicked()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select NordVPN binary"), ui->leNVPNPath->text(),
                                                      QStringLiteral("*.*"));
    if (!path.isEmpty())
        ui->leNVPNPath->setText(path);
}

void Dialog::on_leNVPNPath_textChanged(const QString &text)
{
    ui->leNVPNPath->setToolTip(text);
    QPalette p = ui->leNVPNPath->palette();
    const QColor clr = isValidPath(text) ? ui->leNVPNPath->style()->standardPalette().color(QPalette::Base) : Qt::red;
    if (p.color(QPalette::Base) != clr) {
        p.setColor(QPalette::Base, clr);
        ui->leNVPNPath->setPalette(p);
    }
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
    const QString &path = ui->leNVPNPath->text();
    if (path != AppSettings::Monitor.NVPNPath->read().toString()) {
        if (!isValidPath(path)) {
            QMessageBox::critical(this, tr("NordVPN binary"), tr("Please, specefy valid path."));
            ui->tabWidget->setCurrentIndex(0);
            ui->leNVPNPath->setFocus();

            return false;
        }
        QMessageBox::information(this, tr("NordVPN binary"), tr("Please, restart the application to apply changes."));
    }

    AppSettings::Monitor.NVPNPath->write(path);
    AppSettings::Monitor.Interval->write(ui->spinBoxInterval->value());
    AppSettings::Monitor.Active->write(ui->checkBoxAutoActive->isChecked());

    return true;
}

bool Dialog::saveNVPNSettings()
{
    NIY;
    return true;
}

bool Dialog::isValidPath(const QString &path) const
{
    if (path.isEmpty())
        return false;

    const QFileInfo info(path);
    if (!info.exists())
        return false;

    return info.isExecutable();
}
