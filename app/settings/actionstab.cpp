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

#include "actionstab.h"

#include "actioneditor.h"
#include "actionstorage.h"
#include "common.h"
#include "ui_actionstab.h"

#include <QStandardItemModel>

/*static*/ const int ActionsTab::ActionPointerDataRole = Qt::UserRole + 1;

ActionsTab::ActionsTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionsTab)
    , m_actStorage(nullptr)
    , m_scope(Action::Flow::NordVPN)
{
    ui->setupUi(this);
    ui->buttonRemove->setEnabled(false);

    connect(ui->buttonAdd, &QPushButton::clicked, this, &ActionsTab::onAddRequested);
    connect(ui->buttonRemove, &QPushButton::clicked, this, &ActionsTab::onRemoveRequested);

    ui->listView->setModel(new QStandardItemModel(ui->listView));
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &ActionsTab::onCurrentRowChanged);

    connect(ui->editorWidget, &ActionEditor::titleChanged, this, &ActionsTab::onCurrentTitleChanged);
}

ActionsTab::~ActionsTab()
{
    delete ui;
}

int ActionsTab::actionsCount() const
{
    return m_actionInfos.size();
}

void ActionsTab::setActions(ActionStorage *actStorage, Action::Flow scope)
{
    m_actionInfos.clear();
    model()->removeRows(0, actionsCount());

    m_scope = scope;
    m_actStorage = actStorage;

    ui->editorWidget->prepareUi(m_scope);

    QVector<Action::Ptr> actions;
    switch (m_scope) {
    case Action::Flow::Yangl: {
        ui->buttonAdd->hide();
        ui->buttonRemove->hide();
        delete ui->buttonsLayout;
        actions = m_actStorage->yanglActions();
        break;
    }
    case Action::Flow::NordVPN: {
        ui->buttonAdd->hide();
        ui->buttonRemove->hide();
        delete ui->buttonsLayout;
        actions = m_actStorage->nvpnActions();
        break;
    }
    case Action::Flow::Custom: {
        ui->buttonRemove->setEnabled(actions.size());
        actions = m_actStorage->userActions();
        break;
    }
    }

    for (auto act : actions)
        addAction(act);

    ui->buttonRemove->setEnabled(actions.size());
    ui->listView->setCurrentIndex(model()->index(0, 0));
}

void ActionsTab::addAction(const Action::Ptr &action)
{
    if (!action)
        return;

    m_actionInfos.append(ui->editorWidget->wrapAction(action));

    QStandardItem *item = new QStandardItem(action->title());
    item->setData(QVariant::fromValue(m_actionInfos.last()), ActionPointerDataRole);
    model()->appendRow(item);

    ui->buttonRemove->setEnabled(m_actionInfos.size());
}

void ActionsTab::onAddRequested()
{
    if (Action::Ptr action = m_actStorage->createUserAction(m_actStorage)) {
        action->setTitle(tr("Custom#%1").arg(m_actionInfos.size() + 1));
        addAction(action);

        ui->listView->setCurrentIndex(model()->index(m_actionInfos.size() - 1, 0));
    }
}

void ActionsTab::onRemoveRequested()
{
    const QModelIndex &currId = ui->listView->currentIndex();
    if (!currId.isValid())
        return;

    const int row = currId.row();
    if (const auto &info = currId.data(ActionPointerDataRole).value<ActionEditor::ActionInfoPtr>()) {
        model()->removeRow(row);
        m_actionInfos.removeAll(info);
        if (m_actionInfos.size()) {
            const int nextRow = qBound(0, row, m_actionInfos.size() - 1);
            ui->listView->setCurrentIndex(model()->index(nextRow, 0));
        }
    }

    ui->buttonRemove->setEnabled(m_actionInfos.size());
}

bool ActionsTab::save()
{
    ui->editorWidget->commitInfoHandler();

    QVector<Action::Ptr> actions;
    for (const auto &info : m_actionInfos)
        if (info->apply())
            actions.append(info->m_action);

    return m_actStorage->updateActions(actions, m_scope);
}

void ActionsTab::onCurrentRowChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    ActionEditor::ActionInfoPtr action;
    if (current.isValid())
        action = current.data(ActionPointerDataRole).value<ActionEditor::ActionInfoPtr>();

    ui->editorWidget->setAction(action);
}

void ActionsTab::onCurrentTitleChanged(const QString &title)
{
    const QModelIndex &currRow = ui->listView->selectionModel()->currentIndex();
    if (!currRow.isValid())
        return;

    if (auto info = currRow.data(ActionPointerDataRole).value<ActionEditor::ActionInfoPtr>()) {
        info->m_title = title;
        model()->setData(currRow, title);
    }
}

QStandardItemModel *ActionsTab::model() const
{
    return qobject_cast<QStandardItemModel *>(ui->listView->model());
}
