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

class ActionStorage;
class TestAction : public Action
{
    Q_OBJECT
public:
    TestAction(Action::Flow scope = Action::Flow::Custom, NordVPN action = NordVPN::Unknown, ActionStorage *parent = {},
               const Action::Id &id = {});
    ~TestAction() = default;

private:
    static int MetaIdMenuPlace;
private slots:
    void testCreate_Builtin();
    void testCreate_Custom();

    void checkAction(const Action::Ptr &action, int expectedType, Action::Flow expectedScope,
                     const Action::Id &expectedId = {}) const;

    void testSetTitle();
    void testSetApp();
    void testSetArgs();
    void testSetTimeout();
    void testSetForcedShow();
    void testSetAnchor();
};
