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

#include "actionsfactory.h"

#include "actionaccount.h"
#include "actionconnect.h"
#include "actiondisconnect.h"
#include "actiongroups.h"
#include "actionsettings.h"
#include "actionstatus.h"

/*static*/ Action::Ptr ActionsFactory::createAction(KnownAction action)
{
    switch (action) {
    case KnownAction::CheckStatus:
        return Action::Ptr(new ActionStatus());
    case KnownAction::Connect:
        return Action::Ptr(new ActionConnect());
    case KnownAction::Disconnect:
        return Action::Ptr(new ActionDisconnect());
    case KnownAction::Settings:
        return Action::Ptr(new ActionSettings());
    case KnownAction::Account:
        return Action::Ptr(new ActionAccount());
    case KnownAction::Groups:
        return Action::Ptr(new ActionGroups());
    default:
        return nullptr;
    }
}
