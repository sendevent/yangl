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

#pragma once

#include "statechecker.h"

#include <QDialog>
#include <QSystemTrayIcon>

namespace Ui {
class SettingsDialog;
}

class ActionStorage;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(ActionStorage *actionStorage, QWidget *parent = nullptr);
    ~Dialog() override;

public slots:
    void accept() override;

private:
    Ui::SettingsDialog *ui;
    ActionStorage *m_actStorage;

    bool saveSettings();
    bool saveMonitorSettings();
    bool saveActions();
};
