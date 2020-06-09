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

#include "actioneditor.h"

#include "action.h"
#include "common.h"
#include "ui_actioneditor.h"

ActionEditor::ActionEditor(const Action::Ptr &act, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActionEditor)
    , m_act(nullptr)
{
    ui->setupUi(this);

    ui->spinBoxTimeout->setToolTip(
            QString("%1 — %2").arg(ui->spinBoxTimeout->minimum()).arg(ui->spinBoxTimeout->maximum()));
    connect(ui->leTitle, &QLineEdit::textChanged, this, &ActionEditor::titleChanged);
    setupAction(act);
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

    const bool isCustom = m_act->scope() == Action::Flow::Custom;

    ui->leTitle->setText(m_act->title());
    ui->leApplication->setText(m_act->app());
    ui->leArguments->setText(m_act->args().join(" "));
    ui->spinBoxTimeout->setValue(m_act->timeout() / yangl::OneSecondMs);
    ui->checkBoxForceShow->setChecked(m_act->forcedShow());

    ui->comboBoxMenu->clear();
    ui->comboBoxMenu->addItem(tr("Hide"), static_cast<int>(Action::MenuPlace::NoMenu));
    ui->comboBoxMenu->addItem(tr("Monitor"), static_cast<int>(Action::MenuPlace::Common));
    ui->comboBoxMenu->addItem(isCustom ? tr("Custom") : tr("NordVPN"), static_cast<int>(Action::MenuPlace::Own));
    ui->comboBoxMenu->setCurrentIndex(static_cast<int>(m_act->anchor()));

    QVector<int> excludeRows;
    switch (action->scope()) {
    case Action::Flow::Yangl: {
        excludeRows = { 5, 3, 2, 1, 0 };
        break;
    }
    case Action::Flow::NordVPN: {
        excludeRows = { 1 };
        break;
    }
    default: {
        break;
    }
    }

    for (int row : excludeRows)
        ui->formLayout->removeRow(row);

    if (m_act->scope() == Action::Flow::NordVPN) {
    }
}

bool ActionEditor::apply()
{
    if (m_act) {
        m_act->setTitle(ui->leTitle->text().trimmed());
        const QString &app = ui->leApplication->text().trimmed();
        if (!Action::isValidAppPath(app))
            return false; // TODO: ask
        m_act->setApp(app);
        m_act->setArgs(ui->leArguments->text().split(" ")); // TODO: obey quotes
        m_act->setTimeout(ui->spinBoxTimeout->value());
        m_act->setForcedShow(ui->checkBoxForceShow->isChecked());
        m_act->setAnchor(ui->comboBoxMenu->currentData().value<Action::MenuPlace>());
    }

    return true;
}
