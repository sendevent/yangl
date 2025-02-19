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

#pragma once

#include "app/statechecker.h"

#include <QDialog>
#include <QPointer>
#include <QSystemTrayIcon>

namespace Ui {
class SettingsDialog;
}

class MapSettings;
class ActionStorage;
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    static SettingsDialog *makeVisible(ActionStorage *actionStorage);

    ~SettingsDialog() override;

public slots:
    void accept() override;

private:
    static QPointer<SettingsDialog> m_instance;
    SettingsDialog(ActionStorage *actionStorage, QWidget *parent = {});
    Ui::SettingsDialog *ui;
    ActionStorage *m_actStorage;
    MapSettings *m_mapSettings;

    bool saveSettings();
    bool saveMonitorSettings();
    bool saveActions();
};
