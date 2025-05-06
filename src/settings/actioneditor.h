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
#include "apppatheditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPointer>
#include <QSharedPointer>
#include <QSpinBox>
#include <QWidget>

namespace Ui {
class ActionEditor;
}

class ActionEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ActionEditor(QWidget *parent = {});
    ~ActionEditor();

    void prepareUi(Action::Flow scope);

    struct ActionInfoHandler {
        Action::Ptr m_action;
        QString m_title;
        QString m_app;
        QStringList m_args;
        int m_timeout;
        bool m_forceShow;
        Action::MenuPlace m_menuPlace;

        bool apply();
    };
    using ActionInfoPtr = QSharedPointer<ActionEditor::ActionInfoHandler>;

    static ActionInfoPtr wrapAction(const Action::Ptr &action);
    void setAction(const ActionInfoPtr &actionInfo);
    ActionInfoPtr getAction() const;
    void commitInfoHandler();

signals:
    void titleChanged(const QString &text);

private:
    Ui::ActionEditor *ui;
    ActionEditor::ActionInfoPtr m_actionInfo;

    QPointer<QLineEdit> m_leTitle;
    QPointer<AppPathEditor> m_leApplication;
    QPointer<QLineEdit> m_leArguments;
    QPointer<QSpinBox> m_spinBoxTimeout;
    QPointer<QCheckBox> m_checkBoxForceShow;
    QPointer<QComboBox> m_comboBoxMenu;
};

Q_DECLARE_METATYPE(ActionEditor::ActionInfoPtr);
