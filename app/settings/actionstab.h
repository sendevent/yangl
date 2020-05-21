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

#include "action.h"

#include <QList>
#include <QWidget>

namespace Ui {
class ActionsTab;
}

class ActionStorage;
class ActionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ActionsTab(QWidget *parent = nullptr);
    ~ActionsTab();

    void setActions(ActionStorage *actStorage, Action::Scope scope);

    bool save();

private slots:
    void onAddRequested();
    void onRemoveRequested();

private:
    Ui::ActionsTab *ui;
    QList<Action::Ptr> m_actions;
    ActionStorage *m_actStorage;
    Action::Scope m_scope;

    void addAction(const Action::Ptr &action);
};
