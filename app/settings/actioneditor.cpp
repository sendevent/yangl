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

    m_leTitle = ui->leTitle;
    m_leApplication = ui->leApplication;
    m_leArguments = ui->leArguments;
    m_spinBoxTimeout = ui->spinBoxTimeout;
    m_checkBoxForceShow = ui->checkBoxForceShow;
    m_comboBoxMenu = ui->comboBoxMenu;

    m_spinBoxTimeout->setToolTip(QString("%1 — %2").arg(m_spinBoxTimeout->minimum()).arg(m_spinBoxTimeout->maximum()));
    connect(m_leTitle, &QLineEdit::textChanged, this, &ActionEditor::titleChanged);

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

    m_leTitle->setText(m_act->title());
    m_leApplication->setText(m_act->app());
    m_leArguments->setText(m_act->args().join(" "));
    m_spinBoxTimeout->setValue(m_act->timeout() / yangl::OneSecondMs);
    m_checkBoxForceShow->setChecked(m_act->forcedShow());

    QMap<Action::MenuPlace, QString> anchors { { Action::MenuPlace::NoMenu, tr("Hide") },
                                               { Action::MenuPlace::Common, tr("Common") } };
    QVector<int> excludeRows;
    switch (action->scope()) {
    case Action::Flow::Yangl: {
        excludeRows = { 5, 3, 2, 1, 0 };
        anchors[Action::MenuPlace::Own] = tr("yangl");

        switch (static_cast<Action::Yangl>(action->type())) {
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
        excludeRows = { 1 };
        anchors[Action::MenuPlace::Own] = tr("NordVPN");
        break;
    }
    case Action::Flow::Custom: {
        anchors[Action::MenuPlace::Own] = tr("Extra");
        break;
    }
    }

    for (int row : excludeRows)
        ui->formLayout->removeRow(row);

    m_comboBoxMenu->clear();
    QMap<Action::MenuPlace, int> comboIds;
    for (auto iter = anchors.cbegin(); iter != anchors.cend(); ++iter) {
        const Action::MenuPlace anchor = iter.key();
        comboIds[anchor] = m_comboBoxMenu->count();
        m_comboBoxMenu->addItem(iter.value(), static_cast<int>(anchor));
    }

    m_comboBoxMenu->setCurrentIndex(comboIds.value(m_act->anchor()));
}

bool ActionEditor::apply()
{
    if (m_act) {
        if (m_leTitle)
            m_act->setTitle(m_leTitle->text().trimmed());

        if (m_leApplication) {
            const QString &app = m_leApplication->text().trimmed();
            if (!Action::isValidAppPath(app))
                return false; // TODO: ask
            m_act->setApp(app);
        }

        if (m_leArguments)
            m_act->setArgs(m_leArguments->text().split(" ")); // TODO: obey quotes

        if (m_spinBoxTimeout)
            m_act->setTimeout(m_spinBoxTimeout->value() * yangl::OneSecondMs);

        if (m_checkBoxForceShow)
            m_act->setForcedShow(m_checkBoxForceShow->isChecked());

        if (m_comboBoxMenu)
            m_act->setAnchor(m_comboBoxMenu->currentData().value<Action::MenuPlace>());
    }

    return true;
}
