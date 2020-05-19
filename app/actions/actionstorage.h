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
#include "actionjson.h"

#include <QHash>
#include <QObject>
#include <memory>

class ActionStorage : public QObject
{
    Q_OBJECT

public:
    ActionStorage(QObject *parent = nullptr);

    QList<Action::Ptr> knownActions() const;
    QList<Action::Ptr> userActions() const;
    QList<Action::Ptr> allActions() const;

    Action::Ptr action(int knownAction) const;
    Action::Ptr action(const Action::Id &userAction) const;

    QList<Action::Ptr> load();
    void save();

    Action::Ptr createUserAction();
    bool removeUserAction(const Action::Ptr &action);
    bool updateActions(const QList<Action::Ptr> &actions, Action::ActScope scope);

private:
    QHash<int, Action::Ptr> m_builtinActions;
    QHash<Action::Id, Action::Ptr> m_userActions;
    const std::unique_ptr<ActionJson> m_json;

    void initActions(bool updateFromJson);

    Action::Ptr createBuiltinAction(KnownAction actionType);

    bool updateBuiltinActions(const QList<Action::Ptr> &actions);
    bool updateUserActions(const QList<Action::Ptr> &actions);

    QList<Action::Ptr> sortActionsByTitle(const QList<Action::Ptr> &actions) const;
};
