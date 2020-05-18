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

#include "actionjson.h"
#include "appsettings.h"
#include "settingsmanager.h"

#include <QDebug>
#include <QFile>

#define LOG qDebug() << Q_FUNC_INFO
#define WRN qWarning() << Q_FUNC_INFO
#define NIY WRN << "Not implemented yet"

ActionStorage::ActionStorage(QObject *parent)
    : QObject(parent)
    , m_json(new ActionJson)
{
}

QList<Action::Ptr> ActionStorage::sortActionsByTitle(const QList<Action::Ptr> &actions) const
{
    QList<Action::Ptr> sorted(actions);
    std::sort(sorted.begin(), sorted.end(),
              [](const Action::Ptr &a, const Action::Ptr &b) { return a->title() < b->title(); });
    return sorted;
}

QList<Action::Ptr> ActionStorage::knownActions() const
{
    return sortActionsByTitle(m_builtinActions.values());
}

QList<Action::Ptr> ActionStorage::userActions() const
{
    return sortActionsByTitle(m_userActions.values());
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

void ActionStorage::load()
{
    const bool jsonLoaded = m_json->load();
    initActions(jsonLoaded);
    if (!jsonLoaded)
        m_json->save();
}

Action::Ptr ActionStorage::createUserAction()
{
    Action::Ptr action(new Action(Action::ActScope::User, KnownAction::Unknown, this));
    action->setApp(AppSettings::Monitor.NVPNPath->read().toString());
    action->setArgs({});
    action->setForcedShow(true);
    action->setAnchor(Action::MenuPlace::Own);

    m_userActions.insert(action->id(), action);
    m_json->putAction(action.get());

    return action;
}

bool ActionStorage::removeUserAction(const Action::Ptr &action)
{
    if (!action)
        return false;

    const bool res = m_userActions.remove(action->id());
    if (res)
        m_json->popAction(action.get());
    return res;
}

void ActionStorage::initActions(bool updateFromJson)
{
    m_builtinActions.clear();

    for (int i = KnownAction::Unknown + 1; i < KnownAction::Last; ++i) {
        if (const Action::Ptr &action = createBuiltinAction(static_cast<KnownAction>(i))) {
            m_builtinActions[action->type()] = action;
            if (updateFromJson)
                m_json->updateAction(action.get());
            else
                m_json->putAction(action.get());
        }
    }
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
        forceShow = true;
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Connect: {
        title = QObject::tr("Connect");
        args.append("c");
        forceShow = false;
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Disconnect: {
        title = QObject::tr("Disonnect");
        args.append("disconnect");
        forceShow = false;
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Settings: {
        title = QObject::tr("Show used settings");
        args.append("settings");
        forceShow = true;
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Account: {
        title = QObject::tr("Account details");
        args.append("account");
        forceShow = true;
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Groups: {
        title = QObject::tr("List server groups");
        args.append("groups");
        forceShow = true;
        menuPlace = Action::MenuPlace::Own;
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

bool ActionStorage::updateActions(const QList<Action::Ptr> &actions, Action::ActScope scope)
{
    const bool isBuiltin = scope == Action::ActScope::Builtin;
    return isBuiltin ? updateBuiltinActions(actions) : updateUserActions(actions);
}

void ActionStorage::save()
{
    for (const auto &action : m_builtinActions)
        m_json->putAction(action.get());

    for (const auto &action : m_userActions)
        m_json->putAction(action.get());

    return m_json->save();
}

bool ActionStorage::updateBuiltinActions(const QList<Action::Ptr> &actions)
{
    QList<int> savedActions;
    for (auto action : actions) {
        const int actType = action->type();
        if (m_builtinActions.contains(actType))
            m_builtinActions[actType].swap(action);
        else
            m_builtinActions.insert(actType, action);
        savedActions.append(actType);
    }

    for (const auto key : m_builtinActions.keys()) {
        if (!savedActions.contains(key)) {
            m_builtinActions.remove(key);
        }
    }

    return true;
}

bool ActionStorage::updateUserActions(const QList<Action::Ptr> &actions)
{
    QList<Action::Id> savedActions;
    for (auto action : actions) {
        const Action::Id actType = action->id();
        if (m_userActions.contains(actType))
            m_userActions[actType].swap(action);
        else
            m_userActions.insert(actType, action);
        savedActions.append(actType);
    }

    for (const auto key : m_userActions.keys()) {
        if (!savedActions.contains(key)) {
            m_userActions.remove(key);
        }
    }

    return true;
}
