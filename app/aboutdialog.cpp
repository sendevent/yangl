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

#include "aboutdialog.h"

#include "QApplication"
#include "common.h"
#include "ui_aboutdialog.h"

#include <QDateTime>
#include <QFile>

/*static*/ AboutDialog::Ptr AboutDialog::m_instance = {};

QString readResourceFile(const QString &path)
{
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "Failed opening file:" << path << in.errorString();
        return {};
    }

    return in.readAll();
}

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    const QString &appName = qApp->applicationDisplayName();
    setWindowTitle(tr("About %1").arg(appName));
    QFont f = ui->labelTitle->font();
    f.setPointSize(f.pointSize() + 4);
    ui->labelTitle->setFont(f);
    ui->labelTitle->setText(appName);

    const QString versionTrio = QString("%1.%2.%3").arg(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    ui->labelVersion->setText(tr("Version %1 [%2]").arg(versionTrio, GIT_REV_INFO));

    const QString buildDate = QDateTime::fromSecsSinceEpoch(BUILD_DATE).toUTC().toString("dd MMM yyyy hh:mm:ss t");
    ui->labelBuilt->setText(tr("Built %1").arg(buildDate));

    ui->labelContentAbout->setText(readResourceFile(":/about/resources/about/yangl.html"));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

/*static*/ void AboutDialog::impressTheUser(QWidget *parent)
{
    if (!m_instance) {
        m_instance = new AboutDialog(parent);
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_instance->show();
    m_instance->raise();
    m_instance->activateWindow();
}
