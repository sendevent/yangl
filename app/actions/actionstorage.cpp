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
#include "common.h"

#include <QFileInfo>

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

QList<Action::Ptr> ActionStorage::load(const QString &from)
{
    QString usedPath = from.isEmpty() ? ActionJson::jsonFilePath() : from;
    QFile in(usedPath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "failed opening file:" << usedPath << in.errorString();
        return {};
    }

    return load(&in);
}

QList<Action::Ptr> ActionStorage::load(QIODevice *from)
{
    const bool jsonLoaded = m_json->load(from);
    initActions(jsonLoaded);
    if (!jsonLoaded)
        m_json->save(from);

    return allActions();
}

void ActionStorage::save(const QString &to)
{
    QString usedPath = to.isEmpty() ? ActionJson::jsonFilePath() : to;
    QFile out(usedPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "failed opening file:" << usedPath << out.errorString();
        return;
    }

    save(&out);
}

void ActionStorage::save(QIODevice(*to))
{
    m_json->clear();

    for (const auto &action : m_builtinActions)
        m_json->putAction(action.get());

    for (const auto &action : m_userActions)
        m_json->putAction(action.get());

    return m_json->save(to);
}

Action::Ptr ActionStorage::createUserAction()
{
    Action::Ptr action(new Action(Action::Scope::User, KnownAction::Unknown, this));
    action->setApp(AppSettings::Monitor.NVPNPath->read().toString());
    action->setArgs({});
    action->setForcedShow(true);
    action->setAnchor(Action::MenuPlace::Own);

    return action;
}

void ActionStorage::initActions(bool updateFromJson)
{
    m_builtinActions.clear();

    for (int i = KnownAction::Unknown + 1; i < KnownAction::Last; ++i) {
        if (const Action::Ptr &action = createAction(static_cast<KnownAction>(i))) {
            m_builtinActions.insert(action->type(), action);
            if (updateFromJson)
                m_json->updateAction(action.get());
            else
                m_json->putAction(action.get());
        }
    }

    if (!updateFromJson)
        return;

    m_userActions.clear();
    for (const QString &id : m_json->customActionIds()) {
        if (!id.isEmpty()) {
            if (const Action::Ptr &action = createAction(Unknown, id)) {
                m_json->updateAction(action.get());
                m_userActions.insert(action->id(), action);
            }
        }
    }
}

Action::Ptr ActionStorage::createAction(KnownAction actionType, const QString &id)
{
    const QString &appPath = AppSettings::Monitor.NVPNPath->read().toString();
    Action::Scope scope = Action::Scope::Builtin;

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
    case KnownAction::Pause05: {
        title = QObject::tr("Pause for 5m");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Pause30: {
        title = QObject::tr("Pause for 30m");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Pause60: {
        title = QObject::tr("Pause for 1h");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::PauseCustom: {
        title = QObject::tr("Pause for ?");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Rate5: {
        title = QObject::tr("Rate ★★★★★");
        args.append("rate 5");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Rate4: {
        title = QObject::tr("Rate ★★★★☆");
        args.append("rate 4");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Rate3: {
        title = QObject::tr("Rate ★★★☆☆");
        args.append("rate 3");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case KnownAction::Rate2: {
        title = QObject::tr("Rate ★★☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        args.append("rate 2");
        break;
    }
    case KnownAction::Rate1: {
        title = QObject::tr("Rate ★☆☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        args.append("rate 1");
        break;
    }
    case KnownAction::SetNotifyOff: {
        title = QObject::tr("Notify OFF");
        menuPlace = Action::MenuPlace::Own;
        args.append("set notify 0");
        break;
    }
    case KnownAction::SetNotifyOn: {
        title = QObject::tr("Notify ON");
        menuPlace = Action::MenuPlace::Own;
        args.append("set notify 1");
        break;
    }
    default:
        scope = Action::Scope::User;
        break;
    }

    Action::Ptr action(new Action(scope, actionType, this, id.isEmpty() ? Action::Id() : Action::Id::fromString(id)));
    action->setApp(appPath);
    action->setTitle(title);
    action->setArgs(args);
    action->setForcedShow(forceShow);
    action->setAnchor(menuPlace);

    return action;
}

bool ActionStorage::updateActions(const QList<Action::Ptr> &actions, Action::Scope scope)
{
    const bool isBuiltin = scope == Action::Scope::Builtin;
    return isBuiltin ? updateBuiltinActions(actions) : updateUserActions(actions);
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

    for (const auto key : m_builtinActions.keys())
        if (!savedActions.contains(key))
            m_builtinActions.remove(key);

    return true;
}

bool ActionStorage::updateUserActions(const QList<Action::Ptr> &actions)
{
    QList<Action::Id> savedActions;
    for (auto action : actions) {
        const Action::Id &actId = action->id();
        if (m_userActions.contains(actId)) {
            m_userActions[actId].swap(action);
        } else {
            m_userActions.insert(actId, action);
        }
        savedActions.append(actId);
    }

    for (const auto key : m_userActions.keys())
        if (!savedActions.contains(key)) {
            m_userActions.remove(key);
        }

    return true;
}
