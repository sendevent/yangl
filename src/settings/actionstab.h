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
#include "settings/actioneditor.h"

#include <QList>
#include <QWidget>

namespace Ui {
class ActionsTab;
}

class ActionStorage;
class QStandardItemModel;
class ActionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ActionsTab(QWidget *parent = {});
    ~ActionsTab();

    void setActions(ActionStorage *actStorage, Action::Flow scope);

    bool save();

private slots:
    void onAddRequested();
    void onRemoveRequested();
    void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void onCurrentTitleChanged(const QString &title);

private:
    static const int ActionPointerDataRole;
    Ui::ActionsTab *ui;
    ActionStorage *m_actStorage;
    Action::Flow m_scope;
    QList<ActionEditor::ActionInfoPtr> m_actionInfos;

    void addAction(const Action::Ptr &action);

    int actionsCount() const;
    QStandardItemModel *model() const;
};
