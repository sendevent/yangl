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
    ActionStorage(QObject *parent = {});

    QList<Action::Ptr> knownActions() const;
    QList<Action::Ptr> userActions() const;
    QList<Action::Ptr> allActions() const;

    Action::Ptr action(int knownAction) const;
    Action::Ptr action(const Action::Id &userAction) const;

    QList<Action::Ptr> load(const QString &from = {});
    QList<Action::Ptr> load(QIODevice *from);
    void save(const QString &to = {});
    void save(QIODevice *from);

    Action::Ptr createUserAction(QObject *parent = {});
    Action::Ptr createAction(Action::Scope scope, KnownAction type, const Action::Id &id, const QString &appPath,
                             const QString &title, const QStringList &args, bool alwaysShowResult,
                             Action::MenuPlace anchor, int timeout, QObject *parent);

    bool updateActions(const QList<Action::Ptr> &actions, Action::Scope scope);

    static QString jsonFilePath();

private:
    QHash<int, Action::Ptr> m_builtinActions;
    QHash<Action::Id, Action::Ptr> m_userActions;
    const std::unique_ptr<ActionJson> m_json;

    void initActions(bool updateFromJson);

    Action::Ptr createAction(KnownAction actionType, const QString &id = {});

    bool updateBuiltinActions(const QList<Action::Ptr> &actions);
    bool updateUserActions(const QList<Action::Ptr> &actions);

    QList<Action::Ptr> sortActionsByTitle(const QList<Action::Ptr> &actions) const;

    void loadBuiltinActions();
    void loadUserActions();
};
