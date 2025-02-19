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

#include "apppatheditor.h"

#include "actions/action.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPalette>
#include <QStyle>
#include <QToolButton>

AppPathEditor::AppPathEditor(QWidget *parent)
    : QWidget(parent)
    , m_pathEdit(new QLineEdit(this))
    , m_openFileButton(new QToolButton(this))
{
    QHBoxLayout *hBox(new QHBoxLayout(this));
    hBox->addWidget(m_pathEdit);
    hBox->addWidget(m_openFileButton);

    m_openFileButton->setText(tr("…"));
    connect(m_pathEdit, &QLineEdit::textChanged, this, &AppPathEditor::onAppPathChanged);
    connect(m_openFileButton, &QToolButton::clicked, this, &AppPathEditor::onOpenFileClicked);

    hBox->setContentsMargins(0, 0, 0, 0);
}

QString AppPathEditor::text() const
{
    return m_pathEdit->text();
}

void AppPathEditor::setText(const QString &text)
{
    m_pathEdit->setText(text);
}

void AppPathEditor::onAppPathChanged(const QString &text)
{
    m_pathEdit->setToolTip(text);
    QPalette p = m_pathEdit->palette();
    const QColor clr =
            Action::isValidAppPath(text) ? m_pathEdit->style()->standardPalette().color(QPalette::Base) : Qt::red;
    if (p.color(QPalette::Base) != clr) {
        p.setColor(QPalette::Base, clr);
        m_pathEdit->setPalette(p);
    }
}

void AppPathEditor::onOpenFileClicked()
{
    const QString path =
            QFileDialog::getOpenFileName(this, tr("Select NordVPN binary"), m_pathEdit->text(), tr("Applications (*)"));
    if (!path.isEmpty())
        m_pathEdit->setText(path);
}
