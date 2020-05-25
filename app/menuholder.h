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

#include "action.h"

#include <QMenu>
#include <QObject>
#include <memory>

class MenuHolder : public QObject
{
    Q_OBJECT
public:
    explicit MenuHolder(QObject *parent = nullptr);

    QMenu *createMenu(const QList<Action::Ptr> &actions);

    QAction *getActRun() const;
    QAction *getActShowSettings() const;
    QAction *getActShowMap() const;
    QAction *getActQuit() const;

signals:
    void actionTriggered(Action *action);

private slots:
    void onActionTriggered();

private:
    std::unique_ptr<QMenu> m_menuMonitor;
    QAction *m_actMap;
    QAction *m_actSettings;
    QAction *m_actRun;
    std::unique_ptr<QMenu> m_menuNordVpn;
    std::unique_ptr<QMenu> m_menuUser;
    QAction *m_actSeparatorExit;
    QAction *m_actQuit;

    void populateActions(const QList<Action::Ptr> &actions);
};
