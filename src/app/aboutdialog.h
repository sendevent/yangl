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
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#pragma once

#include <QDialog>
#include <QMap>
#include <QPointer>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    enum TabId
    {
        First = 0,
        Yangl = First,
        License,
        NordVPN,
        Qt,

        Last
    };

    using Ptr = QPointer<AboutDialog>;

    ~AboutDialog();

    static void makeVisible(QWidget *parent);

private:
    static const QMap<AboutDialog::TabId, QString> m_tabs;
    static Ptr m_instance;
    Ui::AboutDialog *ui;
    explicit AboutDialog(QWidget *parent = nullptr);

    QString readResourceFile(const QString &path) const;
    void createTab(TabId tab);
};
