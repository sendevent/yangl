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

#include "actionstorage.h"

#include "actionjson.h"
#include "appsettings.h"
#include "clicall.h"
#include "common.h"

#include <QFileInfo>

ActionStorage::ActionStorage(QObject *parent)
    : QObject(parent)
    , m_json(new ActionJson(this))
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
    return m_builtinActions.value(knownAction, {});
}

Action::Ptr ActionStorage::action(const Action::Id &userAction) const
{
    return m_userActions.value(userAction, {});
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
        m_json->putAction(&*action);

    for (const auto &action : m_userActions)
        m_json->putAction(&*action);

    return m_json->save(to);
}

Action::Ptr ActionStorage::createUserAction(QObject *parent)
{
    return createAction(Action::Scope::User, KnownAction::Unknown, {}, AppSettings::Monitor.NVPNPath->read().toString(),
                        {}, {}, true, Action::MenuPlace::Own, CLICall::DefaultTimeoutMSecs, parent);
}

void ActionStorage::initActions(bool updateFromJson)
{
    if (!updateFromJson) {
        for (int i = KnownAction::Unknown + 1; i < KnownAction::Last; ++i) {
            if (const Action::Ptr &action = createAction(static_cast<KnownAction>(i))) {
                m_builtinActions.insert(action->type(), action);
                m_json->putAction(&*action);
            }
        }
        return;
    }

    loadBuiltinActions();

    loadUserActions();
}

void ActionStorage::loadBuiltinActions()
{
    QMap<KnownAction, Action::Ptr> jsonBuiltinActionsById;
    const QVector<QString> &jsonBuiltinActionIds = m_json->builtinActionIds();
    for (const auto &id : jsonBuiltinActionIds)
        if (const auto &action = m_json->action(Action::Scope::Builtin, id))
            jsonBuiltinActionsById.insert(action->type(), action);

    for (int i = KnownAction::Unknown + 1; i < KnownAction::Last; ++i) {
        const KnownAction actionType = static_cast<KnownAction>(i);
        if (const auto &action = jsonBuiltinActionsById.value(actionType, {})) {
            m_builtinActions[actionType] = action;
            jsonBuiltinActionsById.remove(actionType);
        } else {
            m_builtinActions[actionType] = createAction(actionType);
        }
    }

    while (!jsonBuiltinActionsById.isEmpty()) {
        if (const auto &action = jsonBuiltinActionsById.first()) {
            m_json->popAction(&*action);
            jsonBuiltinActionsById.remove(action->type());
        }
    }

    for (const auto &action : m_builtinActions)
        m_json->putAction(&*action);
}

void ActionStorage::loadUserActions()
{
    for (const QString &id : m_json->customActionIds())
        if (!id.isEmpty())
            if (const Action::Ptr &action = m_json->action(Action::Scope::User, id))
                m_userActions.insert(action->id(), action);
}

Action::Ptr ActionStorage::createAction(KnownAction actionType, const QString &id)
{
    const QString &appPath = AppSettings::Monitor.NVPNPath->read().toString();
    Action::Scope scope = Action::Scope::Builtin;

    QString title;
    QStringList args;
    bool forceShow = false;
    Action::MenuPlace menuPlace = Action::MenuPlace::Own;
    const Action::Id &actId = id.isEmpty() ? Action::Id::createUuid() : Action::Id(id);

    switch (actionType) {
    case KnownAction::CheckStatus: {
        title = QObject::tr("Check status");
        args.append("status");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Connect: {
        title = QObject::tr("Connect");
        args.append("c");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case KnownAction::Disconnect: {
        title = QObject::tr("Disonnect");
        args.append("disconnect");
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
        forceShow = true;
        break;
    }
    case KnownAction::Rate4: {
        title = QObject::tr("Rate ★★★★☆");
        args.append("rate 4");
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case KnownAction::Rate3: {
        title = QObject::tr("Rate ★★★☆☆");
        args.append("rate 3");
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case KnownAction::Rate2: {
        title = QObject::tr("Rate ★★☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        args.append("rate 2");
        forceShow = true;
        break;
    }
    case KnownAction::Rate1: {
        title = QObject::tr("Rate ★☆☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        args.append("rate 1");
        forceShow = true;
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
        forceShow = true;
        break;
    }

    return createAction(scope, actionType, actId, appPath, title, args, forceShow, menuPlace,
                        CLICall::DefaultTimeoutMSecs, this);
}

Action::Ptr ActionStorage::createAction(Action::Scope scope, KnownAction type, const Action::Id &id,
                                        const QString &appPath, const QString &title, const QStringList &args,
                                        bool alwaysShowResult, Action::MenuPlace anchor, int timeout, QObject *parent)
{
    Action::Ptr action;

    if (scope == Action::Scope::Builtin)
        action = m_builtinActions.value(type, {});
    else if (scope == Action::Scope::User && !id.isNull())
        action = m_userActions.value(id, {});

    if (!action)
        action = Action::Ptr(new Action(scope, type, parent, id));

    if (!appPath.isEmpty())
        action->setApp(appPath);
    if (!title.isEmpty())
        action->setTitle(title);
    if (!args.isEmpty())
        action->setArgs(args);
    action->setForcedShow(alwaysShowResult);
    action->setAnchor(anchor);
    if (0 != timeout)
        action->setTimeout(timeout);

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
            m_builtinActions[actType] = action;
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
            m_userActions[actId] = action;
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
