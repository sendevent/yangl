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

#include "actions/action.h"
#include "actions/clicallresultview.h"

#include <QMap>
#include <QPointer>
#include <QWidget>

class QTabWidget;
class ActionResultViewer : public QWidget
{
    Q_OBJECT
public:
    static void makeVisible();
    static void registerAction(Action *action);
    static void unregisterAction(Action *action);
    static void updateLinesLimit();

private slots:
    void onActionStarted(const Action::Id &id, const QString &app, const QStringList &args);
    void onActionPerformed(const Action::Id &id, const QString &result, bool ok, const QString &info);

private:
    static ActionResultViewer *instance();
    static ActionResultViewer *m_instance;
    static int m_linesLimit;

    explicit ActionResultViewer();
    QTabWidget *m_tabWidget;

    QMap<Action::Id, QPointer<Action>> m_actions;
    QMap<Action::Id, QPointer<CLICallResultView>> m_browsers;
    CLICallResultView *displayForAction(Action *action);
};
