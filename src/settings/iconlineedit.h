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

#include <QWidget>

class QLineEdit;
class QToolButton;
class QLabel;

class IconLineEdit : public QWidget
{
    Q_OBJECT
public:
    explicit IconLineEdit(QWidget *parent = nullptr);

    QString path() const;
    void setPath(const QString &path);

private slots:
    void onOpenRequested();
    void onPathChanged();

private:
    QLabel *m_preview;
    QLineEdit *m_edit;
    QToolButton *m_button;

    bool isValidImage(const QString &path) const;
    void updatePreview(const QString &to);
};
