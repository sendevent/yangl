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

#include "actioneditor.h"

#include "actions/action.h"
#include "app/common.h"
#include "ui_actioneditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPointer>
#include <QSharedPointer>
#include <QSpinBox>
#include <QWidget>

ActionEditor::ActionEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionEditor)
    , m_actionInfo(nullptr)
{
    ui->setupUi(this);

    m_leTitle = ui->leTitle;
    m_leApplication = ui->leApplication;
    m_leArguments = ui->leArguments;
    m_spinBoxTimeout = ui->spinBoxTimeout;
    m_checkBoxForceShow = ui->checkBoxForceShow;
    m_comboBoxMenu = ui->comboBoxMenu;

    m_spinBoxTimeout->setToolTip(QString("%1 — %2").arg(m_spinBoxTimeout->minimum()).arg(m_spinBoxTimeout->maximum()));
    connect(m_leTitle, &QLineEdit::textChanged, this, &ActionEditor::titleChanged);
}

ActionEditor::~ActionEditor()
{
    delete ui;
}

void ActionEditor::prepareUi(Action::Flow scope)
{
    static const QMap<Action::Flow, QList<int>> excludedRows {
        { Action::Flow::Yangl, { 5, 3, 2, 1, 0 } },
        { Action::Flow::NordVPN, { 1 } },
        { Action::Flow::Custom, {} },
    };

    const auto &collection = excludedRows[scope];
    for (int i : collection) {
        ui->formLayout->removeRow(i);
    }
}

bool ActionEditor::ActionInfoHandler::apply()
{
    if (!m_action)
        return false;

    m_action->setTitle(m_title.trimmed());
    m_action->setApp(m_app);
    m_action->setArgs(m_args); // TODO: obey quotes
    m_action->setTimeout(m_timeout);
    m_action->setForcedShow(m_forceShow);
    m_action->setAnchor(m_menuPlace);

    return true;
}

/*static*/ ActionEditor::ActionInfoPtr ActionEditor::wrapAction(const Action::Ptr &action)
{
    if (!action)
        return {};

    ActionEditor::ActionInfoPtr info(new ActionEditor::ActionInfoHandler);

    info->m_action = action;
    info->m_title = action->title();
    info->m_app = action->app();
    info->m_args = action->args();
    info->m_timeout = action->timeout();
    info->m_forceShow = action->forcedShow();
    info->m_menuPlace = action->anchor();

    return info;
}

void ActionEditor::setAction(const ActionInfoPtr &actionInfo)
{
    if (m_actionInfo)
        commitInfoHandler();

    m_actionInfo = actionInfo;

    for (int i = 0; i < ui->formLayout->count(); ++i)
        ui->formLayout->itemAt(i)->widget()->setVisible(m_actionInfo.get());

    if (!m_actionInfo || !m_actionInfo->m_action)
        return;

    if (m_leTitle)
        m_leTitle->setText(m_actionInfo->m_title);
    if (m_leApplication)
        m_leApplication->setText(m_actionInfo->m_app);
    if (m_leArguments)
        m_leArguments->setText(m_actionInfo->m_args.join(" "));
    if (m_spinBoxTimeout)
        m_spinBoxTimeout->setValue(m_actionInfo->m_timeout / utils::oneSecondMs());
    if (m_checkBoxForceShow)
        m_checkBoxForceShow->setChecked(m_actionInfo->m_forceShow);

    if (!m_comboBoxMenu)
        return;

    QMap<Action::MenuPlace, QString> anchors { { Action::MenuPlace::NoMenu, tr("Hide") },
                                               { Action::MenuPlace::Common, tr("Common") } };
    // QList<int> excludeRows;
    switch (m_actionInfo->m_action->scope()) {
    case Action::Flow::Yangl: {
        anchors[Action::MenuPlace::Own] = tr("yangl");
        switch (static_cast<Action::Yangl>(m_actionInfo->m_action->type())) {
        case Action::Yangl::ShowSettings:
        case Action::Yangl::Quit:
            anchors.remove(Action::MenuPlace::NoMenu);
            break;
        default:
            break;
        }

        break;
    }
    case Action::Flow::NordVPN: {
        anchors[Action::MenuPlace::Own] = tr("NordVPN");
        break;
    }
    case Action::Flow::Custom: {
        anchors[Action::MenuPlace::Own] = tr("Extra");
        break;
    }
    }

    m_comboBoxMenu->clear();
    QMap<Action::MenuPlace, int> comboIds;
    for (auto iter = anchors.cbegin(); iter != anchors.cend(); ++iter) {
        const Action::MenuPlace anchor = iter.key();
        comboIds[anchor] = m_comboBoxMenu->count();
        m_comboBoxMenu->addItem(iter.value(), static_cast<int>(anchor));
    }

    m_comboBoxMenu->setCurrentIndex(comboIds.value(m_actionInfo->m_menuPlace));
}

ActionEditor::ActionInfoPtr ActionEditor::getAction() const
{
    return m_actionInfo;
}

void ActionEditor::commitInfoHandler()
{
    if (!m_actionInfo)
        return;

    if (m_leTitle)
        m_actionInfo->m_title = m_leTitle->text().trimmed();

    if (m_leApplication)
        m_actionInfo->m_app = m_leApplication->text().trimmed();

    if (m_leArguments)
        m_actionInfo->m_args = m_leArguments->text().trimmed().split(' ');

    if (m_spinBoxTimeout)
        m_actionInfo->m_timeout = m_spinBoxTimeout->value() * utils::oneSecondMs();

    if (m_checkBoxForceShow)
        m_actionInfo->m_forceShow = m_checkBoxForceShow->isChecked();

    if (m_comboBoxMenu)
        m_actionInfo->m_menuPlace = m_comboBoxMenu->currentData().value<Action::MenuPlace>();
}
