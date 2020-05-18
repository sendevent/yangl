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

#include "actionstab.h"

#include "actioneditor.h"
#include "ui_actionstab.h"

ActionsTab::ActionsTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionsTab)
    , m_actions()
{
    ui->setupUi(this);
    ui->buttonRemove->setEnabled(false);
}

ActionsTab::~ActionsTab()
{
    delete ui;
}

void ActionsTab::setActions(const QList<Action::Ptr> &actions, Action::ActScope scope)
{
    while (ui->toolBox->count()) {
        const int pos = ui->toolBox->count() - 1;
        auto last = ui->toolBox->widget(pos);
        ui->toolBox->removeItem(pos);
        delete last;
    }

    m_actions = actions;
    if (scope == Action::ActScope::Builtin) {
        ui->buttonAdd->hide();
        ui->buttonRemove->hide();
        delete ui->buttonsLayout;
    } else
        ui->buttonRemove->setEnabled(m_actions.size());

    for (auto act : m_actions) {
        ActionEditor *editor = new ActionEditor(act.get());
        connect(editor, &ActionEditor::titleChanged, this, [this, editor](const QString &title) {
            ui->toolBox->setItemText(ui->toolBox->indexOf(editor), title);
        });
        ui->toolBox->addItem(editor, act->title());
    }
}
