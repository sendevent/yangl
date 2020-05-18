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

#include "actioneditor.h"

#include "action.h"
#include "ui_actioneditor.h"

ActionEditor::ActionEditor(const Action::Ptr &act, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionEditor)
    , m_act(nullptr)
{
    ui->setupUi(this);

    ui->spinBoxTimeout->setToolTip(
            QString("%1 — %2").arg(ui->spinBoxTimeout->minimum()).arg(ui->spinBoxTimeout->maximum()));
    setupAction(act);

    connect(ui->leTitle, &QLineEdit::textChanged, this, &ActionEditor::titleChanged);
}

ActionEditor::~ActionEditor()
{
    delete ui;
}

Action::Ptr ActionEditor::getAction() const
{
    return m_act;
}

void ActionEditor::setupAction(const Action::Ptr &action)
{
    m_act = action;
    if (!m_act)
        return;

    const bool isCustom = m_act->actionScope() == Action::ActScope::User;

    ui->leTitle->setText(m_act->title());
    ui->leApplication->setText(m_act->app());
    ui->leArguments->setText(m_act->args().join(" "));
    ui->spinBoxTimeout->setValue(m_act->timeout() / 1000);
    ui->checkBoxForceShow->setChecked(m_act->forcedShow());

    ui->comboBoxMenu->clear();
    ui->comboBoxMenu->addItem(tr("Hide"), Action::MenuPlace::NoMenu);
    ui->comboBoxMenu->addItem(tr("Monitor"), Action::MenuPlace::Common);
    ui->comboBoxMenu->addItem(isCustom ? tr("Custom") : tr("NordVPN"),
                              isCustom ? Action::MenuPlace::Common : Action::MenuPlace::Own);
    ui->comboBoxMenu->setCurrentIndex(m_act->menuPlace());

    if (m_act->actionScope() == Action::ActScope::Builtin) {
        ui->labelApp->hide();
        ui->leApplication->hide();
        ui->btnApplication->hide();
        ui->formLayout->removeWidget(ui->labelApp);
        ui->formLayout->removeWidget(ui->hLayoutApp->widget());
    }
}
