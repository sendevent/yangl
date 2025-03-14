/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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

    QList<Action::Ptr> yanglActions() const;
    QList<Action::Ptr> nvpnActions() const;
    QList<Action::Ptr> userActions() const;
    QList<Action::Ptr> allActions() const;

    Action::Ptr action(Action::Yangl yanglAction) const;
    Action::Ptr action(Action::NordVPN knownAction) const;
    Action::Ptr action(const Action::Id &userAction) const;

    QList<Action::Ptr> load(const QString &from = {});
    QList<Action::Ptr> load(QIODevice *from);
    void save(const QString &to = {});
    void save(QIODevice *from);

    Action::Ptr createUserAction(QObject *parent = {});
    Action::Ptr createAction(Action::Flow scope, int type, const Action::Id &id, const QString &appPath,
                             const QString &title, const QStringList &args, bool alwaysShowResult,
                             Action::MenuPlace anchor, int timeout, QObject *parent);

    bool updateActions(const QList<Action::Ptr> &actions, Action::Flow scope);

private:
    QHash<Action::Yangl, Action::Ptr> m_yanglActions;
    QHash<Action::NordVPN, Action::Ptr> m_nvpnActions;
    QHash<Action::Id, Action::Ptr> m_userActions;

    const std::unique_ptr<ActionJson> m_json;

    void loadActions();

    Action::Ptr createAction(Action::Flow flow, int actionType, const QString &id = {});

    Action::Ptr createYanglAction(Action::Yangl actionType, const QString &id = {});
    Action::Ptr createNVPNAction(Action::NordVPN actionType, const QString &id = {});

    bool updateBuiltinActions(const QList<Action::Ptr> &actions);
    bool updateUserActions(const QList<Action::Ptr> &actions);

    QList<Action::Ptr> sortActionsByTitle(const QList<Action::Ptr> &actions) const;

    void loadYanglActions();
    void loadBuiltinActions();
    void loadUserActions();
};
