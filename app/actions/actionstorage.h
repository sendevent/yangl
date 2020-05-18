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

#include <QHash>
#include <QJsonObject>
#include <QObject>

class ActionStorage : public QObject
{
    Q_OBJECT

public:
    ActionStorage(QObject *parent = nullptr);
    ~ActionStorage() = default;

    QList<Action::Ptr> knownActions() const;
    QList<Action::Ptr> userActions() const;
    QList<Action::Ptr> allActions() const;

    Action::Ptr action(int knownAction) const;
    Action::Ptr action(const Action::Id &userAction) const;

    void initBuiltinActions();

    Action::Ptr createUserAction();
    bool removeUserAction(const Action::Ptr &action);
    bool save(const QList<Action::Ptr> actions, Action::ActScope scope);

private slots:
    void onActionChanged();

private:
    QHash<int, Action::Ptr> m_builtinActions;
    QHash<Action::Id, Action::Ptr> m_userActions;
    QJsonObject m_json;

    void initActions();
    void loadUserActions();
    void saveUserActions();

    Action::Ptr createBuiltinAction(KnownAction actionType);

    bool builtinActionShowForced(KnownAction action, bool defaultValue) const;
    Action::MenuPlace builtinActionMenuPlace(KnownAction action, Action::MenuPlace defaultPlace);

    QJsonObject actionToJson(Action *action) const;
};
