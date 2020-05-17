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

#include "appsettings.h"

/*static*/ Action::Ptr ActionsFactory::createAction(KnownAction actionType)
{
    const QString &appPath = AppSettings::Monitor.NVPNPath->read().toString();

    switch (actionType) {
    case KnownAction::CheckStatus: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::CheckStatus));
        action->m_title = QObject::tr("Check status");
        action->m_app = appPath;
        action->m_args.append("status");
        action->m_forceShow = true;
        action->m_menuPlace = Action::MenuPlace::Common;
        return action;
    }
    case KnownAction::Connect: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::Connect));
        action->m_title = QObject::tr("Connect");
        action->m_app = appPath;
        action->m_args.append("c");
        action->m_forceShow = false;
        action->m_menuPlace = Action::MenuPlace::Common;
        return action;
    }
    case KnownAction::Disconnect: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::Disconnect));
        action->m_title = QObject::tr("Disonnect");
        action->m_app = appPath;
        action->m_args.append("disconnect");
        action->m_forceShow = false;
        action->m_menuPlace = Action::MenuPlace::Own;
        return action;
    }
    case KnownAction::Settings: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::Settings));
        action->m_title = QObject::tr("Show used settings");
        action->m_app = appPath;
        action->m_args.append("settings");
        action->m_forceShow = true;
        action->m_menuPlace = Action::MenuPlace::Own;
        return action;
    }
    case KnownAction::Account: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::Account));
        action->m_title = QObject::tr("Account details");
        action->m_app = appPath;
        action->m_args.append("account");
        action->m_forceShow = true;
        action->m_menuPlace = Action::MenuPlace::Own;
        return action;
    }
    case KnownAction::Groups: {
        Action::Ptr action(new Action(Action::ActScope::Builtin, KnownAction::Groups));
        action->m_title = QObject::tr("List server groups");
        action->m_app = appPath;
        action->m_args.append("groups");
        action->m_forceShow = true;
        action->m_menuPlace = Action::MenuPlace::Own;
        return action;
    }
    default:
        return nullptr;
    }
}
