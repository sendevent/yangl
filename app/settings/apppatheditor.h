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

#pragma once

#include <QObject>
#include <QWidget>

class QLineEdit;
class QToolButton;
class AppPathEditor : public QWidget
{
    Q_OBJECT
public:
    explicit AppPathEditor(QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString &text);
signals:

private slots:
    void onAppPathChanged(const QString &text);
    void onOpenFileClicked();

private:
    QLineEdit *m_pathEdit;
    QToolButton *m_openFileButton;
};
