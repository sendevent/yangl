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

#include "actionsfactory.h"

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
        if (const Action::Ptr &action = ActionsFactory::createAction(static_cast<KnownAction>(i)))
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
