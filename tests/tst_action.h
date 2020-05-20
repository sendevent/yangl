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

class ActionStorage;
class tst_Action : public Action
{
    Q_OBJECT
public:
    tst_Action(Action::ActScope scope = Action::ActScope::User, KnownAction action = KnownAction::Unknown,
               ActionStorage *parent = nullptr, const Action::Id &id = {});
    ~tst_Action() = default;

private slots:
    void testCreate_Builtin();
    void testCreate_Custom();

    void checkAction(const Action::Ptr &action, KnownAction expectedType, Action::ActScope expectedScope,
                     const Action::Id &expectedId = {}) const;

    void testSetTitle();
    void testSetApp();
    void testSetArgs();
    void testSetTimeout();
    void testSetForcedShow();
    void testSetAnchor();
};
