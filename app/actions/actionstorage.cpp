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

#include "actionstorage.h"

#include "appsettings.h"

#include <QDebug>

#define LOG qDebug() << Q_FUNC_INFO
#define NIY qWarning() << Q_FUNC_INFO << "Not implemented yet"

ActionStorage::ActionStorage(QObject *parent)
    : QObject(parent)
{
}

QList<Action::Ptr> ActionStorage::knownActions() const
{
    return m_builtinActions.values();
}

QList<Action::Ptr> ActionStorage::userActions() const
{
    return m_userActions.values();
}

QList<Action::Ptr> ActionStorage::allActions() const
{
    return knownActions() + userActions();
}

Action::Ptr ActionStorage::action(int knownAction) const
{
    return m_builtinActions.value(knownAction, nullptr);
}

Action::Ptr ActionStorage::action(const Action::Id &userAction) const
{
    return m_userActions.value(userAction, nullptr);
}

void ActionStorage::initActions()
{
    initBuiltinActions();
    loadUserActions();
}

void ActionStorage::initBuiltinActions()
{
    m_builtinActions.clear();

    for (int i = KnownAction::Unknown + 1; i < KnownAction::Last; ++i)
        if (const Action::Ptr &action = createBuiltinAction(static_cast<KnownAction>(i)))
            m_builtinActions[action->type()] = action;
}

void ActionStorage::loadUserActions()
{
    NIY;
}

void ActionStorage::saveUserActions()
{
    NIY;
}

bool ActionStorage::builtinActionShowForced(KnownAction action, bool defaultValue) const
{
    NIY << action;
    return defaultValue;
}

Action::MenuPlace ActionStorage::builtinActionMenuPlace(KnownAction action, Action::MenuPlace defaultPlace)
{
    NIY << action;
    return defaultPlace;
}

Action::Ptr ActionStorage::createBuiltinAction(KnownAction actionType)
{
    const QString &appPath = AppSettings::Monitor.NVPNPath->read().toString();
    const Action::ActScope scope = Action::ActScope::Builtin;

    QString title;
    QStringList args;
    bool forceShow = true;
    Action::MenuPlace menuPlace = Action::MenuPlace::Own;

    switch (actionType) {
    case KnownAction::CheckStatus: {
        title = QObject::tr("Check status");
        args.append("status");
        forceShow = builtinActionShowForced(actionType, true);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Common);
        break;
    }
    case KnownAction::Connect: {
        title = QObject::tr("Connect");
        args.append("c");
        forceShow = builtinActionShowForced(actionType, false);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Common);
        break;
    }
    case KnownAction::Disconnect: {
        title = QObject::tr("Disonnect");
        args.append("disconnect");
        forceShow = builtinActionShowForced(actionType, false);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Own);
        break;
    }
    case KnownAction::Settings: {
        title = QObject::tr("Show used settings");
        args.append("settings");
        forceShow = builtinActionShowForced(actionType, true);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Own);
        break;
    }
    case KnownAction::Account: {
        title = QObject::tr("Account details");
        args.append("account");
        forceShow = builtinActionShowForced(actionType, true);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Own);
        break;
    }
    case KnownAction::Groups: {
        title = QObject::tr("List server groups");
        args.append("groups");
        forceShow = builtinActionShowForced(actionType, true);
        menuPlace = builtinActionMenuPlace(actionType, Action::MenuPlace::Own);
        break;
    }
    default:
        return nullptr;
    }

    Action::Ptr action(new Action(scope, actionType, this));
    action->setApp(appPath);
    action->setTitle(title);
    action->setArgs(args);
    action->setForcedShow(forceShow);
    action->setAnchor(menuPlace);
    return action;
}
