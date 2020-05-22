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

#include "tst_action.h"

#include <QObject>

class ActionStorage;
class tst_ActionStorage : public QObject
{
    Q_OBJECT
public:
    explicit tst_ActionStorage(QObject *parent = nullptr);

private:
    QList<Action::Ptr> populateUserActions(ActionStorage *storage, int count);

private slots:
    void init();
    void cleanup();

    void test_builtinActions();
    void test_userActions();
    void test_allActions();
    void test_actionBuiltin();
    void test_actionUser();
    void test_saveAndLoad();
    void test_createUserAction();
    void test_updateActionsBuiltin();
    void test_updateActionsUser();
};
