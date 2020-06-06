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
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "aboutdialog.h"

#include "QApplication"
#include "common.h"
#include "ui_aboutdialog.h"

#include <QDateTime>
#include <QFile>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

/*static*/ const QMap<AboutDialog::TabId, QString> AboutDialog::m_tabs = {
    { AboutDialog::TabId::Yangl, QObject::tr("yangl.html") },
    { AboutDialog::TabId::License, QObject::tr("License.txt") },
    { AboutDialog::TabId::NordVPN, QObject::tr("NordVPN.html") },
    { AboutDialog::TabId::Qt, QObject::tr("Qt.html") },
};

/*static*/ AboutDialog::Ptr AboutDialog::m_instance = {};

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
    ui->labelVersion->setText(tr("Version %1 %2").arg(versionTrio, GIT_REV_INFO));

    const QString buildDate = QDateTime::fromSecsSinceEpoch(BUILD_DATE).toUTC().toString("dd MMM yyyy hh:mm:ss t");
    ui->labelBuilt->setText(tr("Built %1").arg(buildDate));

    for (int i = TabId::Yangl; i < TabId::Last; ++i)
        createTab(static_cast<AboutDialog::TabId>(i));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

/*static*/ void AboutDialog::makeVisible(QWidget *parent)
{
    if (!m_instance) {
        m_instance = new AboutDialog(parent);
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_instance->show();
    m_instance->raise();
    m_instance->activateWindow();
}

QString AboutDialog::readResourceFile(const QString &path) const
{
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString &msg = tr("Failed opening file \"%1\": %2").arg(path, in.errorString());
        WRN << msg;
        return msg;
    }

    return in.readAll();
}

void AboutDialog::createTab(TabId tabId)
{
    const QString &file = AboutDialog::m_tabs[tabId];
    const QStringList nameParts = file.split('.');

    QWidget *tab = new QWidget(ui->tabWidget);
    QVBoxLayout *vBox = new QVBoxLayout(tab);
    QTextBrowser *display = new QTextBrowser(tab);
    vBox->addWidget(display);
    ui->tabWidget->addTab(tab, nameParts.first());

    display->setOpenExternalLinks(true);
    const QString &content = readResourceFile(QStringLiteral(":/about/resources/about/%1").arg(file));
    if (nameParts.last() == QStringLiteral("txt"))
        display->setPlainText(content);
    else
        display->setHtml(content);

    switch (tabId) {
    case TabId::Qt: {
        QPushButton *btnAboutQt = new QPushButton(tr("About Qt"), tab);
        connect(btnAboutQt, &QPushButton::clicked, qApp, &QApplication::aboutQt);
        vBox->addWidget(btnAboutQt);
        break;
    }
    default:
        break;
    }
}
