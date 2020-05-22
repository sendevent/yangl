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
#include "actionstorage.h"
#include "ui_actionstab.h"

ActionsTab::ActionsTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionsTab)
    , m_actions()
    , m_actStorage(nullptr)
    , m_scope(Action::Scope::Builtin)
{
    ui->setupUi(this);
    ui->buttonRemove->setEnabled(false);

    connect(ui->buttonAdd, &QPushButton::clicked, this, &ActionsTab::onAddRequested);
    connect(ui->buttonRemove, &QPushButton::clicked, this, &ActionsTab::onRemoveRequested);
}

ActionsTab::~ActionsTab()
{
    delete ui;
}

void ActionsTab::setActions(ActionStorage *actStorage, Action::Scope scope)
{
    m_scope = scope;
    m_actStorage = actStorage;
    while (ui->toolBox->count()) {
        const int pos = ui->toolBox->count() - 1;
        auto last = ui->toolBox->widget(pos);
        ui->toolBox->removeItem(pos);
        delete last;
    }

    if (scope == Action::Scope::Builtin) {
        ui->buttonAdd->hide();
        ui->buttonRemove->hide();
        delete ui->buttonsLayout;
        m_actions = m_actStorage->knownActions();
    } else {
        ui->buttonRemove->setEnabled(m_actions.size());
        m_actions = m_actStorage->userActions();
    }

    for (auto act : m_actions)
        addAction(act);
}

void ActionsTab::addAction(const Action::Ptr &action)
{
    if (!action)
        return;

    ActionEditor *editor = new ActionEditor(action);
    connect(editor, &ActionEditor::titleChanged, this,
            [this, editor](const QString &title) { ui->toolBox->setItemText(ui->toolBox->indexOf(editor), title); });
    ui->toolBox->addItem(editor, action->title());
    ui->buttonRemove->setEnabled(m_actions.size());
}

void ActionsTab::onAddRequested()
{
    if (Action::Ptr action = m_actStorage->createUserAction()) {
        action->setTitle(tr("Custom#%1").arg(m_actions.size() + 1));
        m_actions.append(action);
        addAction(action);
    }
}

void ActionsTab::onRemoveRequested()
{
    if (auto editor = qobject_cast<ActionEditor *>(ui->toolBox->currentWidget())) {
        if (m_actStorage->removeUserAction(editor->getAction())) {
            const int id = ui->toolBox->currentIndex();
            ui->toolBox->removeItem(id);
            editor->deleteLater();
            m_actions.removeAt(id);
            ui->buttonRemove->setEnabled(m_actions.size());
        }
    }
}

bool ActionsTab::save()
{
    QList<Action::Ptr> actions;
    for (int i = 0; i < ui->toolBox->count(); ++i) {
        if (auto editor = qobject_cast<ActionEditor *>(ui->toolBox->widget(i))) {
            if (!editor->apply())
                return false;
            actions.append(editor->getAction());
        }
    }

    return m_actStorage->updateActions(actions, m_scope);
}
