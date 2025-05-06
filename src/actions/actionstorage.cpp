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
#include "app/common.h"
#include "cli/clicall.h"
#include "settings/appsettings.h"

#include <QFileInfo>
#include <quuid.h>
#include <utility>

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

QList<Action::Ptr> ActionStorage::yanglActions() const
{
    return sortActionsByTitle(m_yanglActions.values());
}

QList<Action::Ptr> ActionStorage::nvpnActions() const
{
    return sortActionsByTitle(m_nvpnActions.values());
}

QList<Action::Ptr> ActionStorage::userActions() const
{
    return sortActionsByTitle(m_userActions.values());
}

QList<Action::Ptr> ActionStorage::allActions() const
{
    return yanglActions() + nvpnActions() + userActions();
}

Action::Ptr ActionStorage::action(Action::NordVPN requested) const
{
    const auto &collection = nvpnActions();
    auto found = std::find_if(collection.cbegin(), collection.cend(), [&requested](const Action::Ptr &action) {
        return static_cast<Action::NordVPN>(action->type()) == requested;
    });
    return found == collection.end() ? nullptr : *found;
}

Action::Ptr ActionStorage::action(const Action::Id &requested) const
{
    const auto &collection = userActions();
    auto found = std::find_if(collection.cbegin(), collection.cend(),
                              [&requested](const Action::Ptr &action) { return action->id() == requested; });
    return found == collection.end() ? nullptr : *found;
}

Action::Ptr ActionStorage::action(Action::Yangl requested) const
{
    const auto &collection = yanglActions();
    auto found = std::find_if(collection.cbegin(), collection.cend(), [&requested](const Action::Ptr &action) {
        return static_cast<Action::Yangl>(action->type()) == requested;
    });
    return found == collection.end() ? nullptr : *found;
}

QList<Action::Ptr> ActionStorage::load(const QString &from)
{
    const auto &usedPath = from.isEmpty() ? ActionJson::jsonFilePath() : from;
    QFile in(usedPath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "failed opening file:" << usedPath << in.errorString();
    }

    return load(&in);
}

QList<Action::Ptr> ActionStorage::load(QIODevice *from)
{
    const bool jsonLoaded = m_json->load(from);
    loadActions();
    if (!jsonLoaded)
        m_json->save(from);

    return allActions();
}

void ActionStorage::save(const QString &to)
{
    const auto &usedPath = to.isEmpty() ? ActionJson::jsonFilePath() : to;
    QFile out(usedPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "failed opening file:" << usedPath << out.errorString();
        return;
    }

    save(&out);
}

void ActionStorage::save(QIODevice *to)
{
    m_json->clear();

    for (const auto &actionsCollection : { yanglActions(), nvpnActions(), userActions() }) {
        for (const auto &action : actionsCollection) {
            m_json->putAction(action.get());
        }
    }

    return m_json->save(to);
}

Action::Ptr ActionStorage::createUserAction(QObject *parent)
{
    return createAction(Action::Flow::Custom, 0, {}, AppSettings::Monitor->NVPNPath->read().toString(), {}, {}, true,
                        Action::MenuPlace::Own, CLICall::DefaultTimeoutMSecs, parent);
}

void ActionStorage::loadActions()
{
    loadYanglActions();

    loadBuiltinActions();

    loadUserActions();
}

void ActionStorage::loadYanglActions()
{
    QMap<Action::Yangl, Action::Ptr> jsonYanglActionsById;
    const QList<QString> &jsonBuiltinActionIds = m_json->yanglActionIds();
    for (const auto &id : jsonBuiltinActionIds)
        if (const auto &action = m_json->action(Action::Flow::Yangl, id))
            jsonYanglActionsById.insert(static_cast<Action::Yangl>(action->type()), action);

    const auto &actionTypes = Action::yanglActions();
    for (auto actionType : actionTypes) {
        if (const auto &action = jsonYanglActionsById.value(actionType, {})) {
            m_yanglActions[actionType] = action;
            jsonYanglActionsById.remove(actionType);
        } else {
            m_yanglActions[actionType] = createYanglAction(actionType);
        }
    }

    while (!jsonYanglActionsById.isEmpty()) {
        if (const auto &action = jsonYanglActionsById.first()) {
            m_json->popAction(action.get());
            jsonYanglActionsById.remove(static_cast<Action::Yangl>(action->type()));
        }
    }

    for (const auto &action : std::as_const(m_nvpnActions)) {
        m_json->putAction(action.get());
    }
}

void ActionStorage::loadBuiltinActions()
{
    QMap<Action::NordVPN, Action::Ptr> jsonBuiltinActionsById;
    const QList<QString> &jsonBuiltinActionIds = m_json->builtinActionIds();
    for (const auto &id : jsonBuiltinActionIds) {
        if (const auto &action = m_json->action(Action::Flow::NordVPN, id)) {
            jsonBuiltinActionsById.insert(static_cast<Action::NordVPN>(action->type()), action);
        }
    }

    const auto &actions = Action::nvpnActions();
    for (auto actionType : actions) {
        if (const auto &action = jsonBuiltinActionsById.value(actionType, {})) {
            m_nvpnActions[actionType] = action;
            jsonBuiltinActionsById.remove(actionType);
        } else {
            m_nvpnActions[actionType] = createNVPNAction(actionType);
        }
    }

    while (!jsonBuiltinActionsById.isEmpty()) {
        if (const auto &action = jsonBuiltinActionsById.first()) {
            m_json->popAction(action.get());
            jsonBuiltinActionsById.remove(static_cast<Action::NordVPN>(action->type()));
        }
    }

    for (const auto &action : std::as_const(m_nvpnActions)) {
        m_json->putAction(action.get());
    }
}

void ActionStorage::loadUserActions()
{
    const auto &ids = m_json->customActionIds();
    for (const QString &id : ids) {
        if (!id.isEmpty()) {
            if (const Action::Ptr &action = m_json->action(Action::Flow::Custom, id)) {
                m_userActions.insert(action->id(), action);
            }
        }
    }
}

Action::Ptr ActionStorage::createAction(Action::Flow flow, int actionType, const QString &id)
{
    switch (flow) {
    case Action::Flow::Yangl:
        return createYanglAction(static_cast<Action::Yangl>(actionType), id);
    case Action::Flow::NordVPN:
        return createNVPNAction(static_cast<Action::NordVPN>(actionType), id);
    default:
        return createUserAction(this);
    }
}

Action::Ptr ActionStorage::createYanglAction(Action::Yangl actionType, const QString &id)
{
    QString title;
    Action::MenuPlace anchor = Action::MenuPlace::Own;
    switch (actionType) {
    case Action::Yangl::ShowMap:
        title = tr("Show Map");
        break;
    case Action::Yangl::ShowSettings:
        title = tr("Settings");
        break;
    case Action::Yangl::ShowLog:
        title = tr("Log");
        break;
    case Action::Yangl::Activated:
        title = tr("Active");
        anchor = Action::MenuPlace::Common;
        break;
    case Action::Yangl::ShowAbout:
        title = tr("About");
        break;
    case Action::Yangl::Quit:
        title = tr("Quit");
        anchor = Action::MenuPlace::Common;
        break;
    }

    const Action::Flow scope = Action::Flow::Yangl;
    const int t = static_cast<int>(actionType);
    const Action::Id &i = QUuid::fromString(id);
    const QString appPath = {};
    const QStringList args = {};
    bool alwaysShowResult = false;
    const int timeout = CLICall::DefaultTimeoutMSecs;

    return createAction(scope, t, i, appPath, title, args, alwaysShowResult, anchor, timeout, this);
}

Action::Ptr ActionStorage::createNVPNAction(Action::NordVPN actionType, const QString &id)
{
    const QString &appPath = AppSettings::Monitor->NVPNPath->read().toString();
    Action::Flow scope = Action::Flow::NordVPN;

    QString title;
    QStringList args;
    bool forceShow = false;
    Action::MenuPlace menuPlace = Action::MenuPlace::Own;
    const Action::Id &actId = id.isEmpty() ? Action::Id::createUuid() : Action::Id(id);
    auto wordsToList = [&args](const QString &noQuotes) { args << noQuotes.split(' '); };

    switch (actionType) {
    case Action::NordVPN::LogIn: {
        title = QObject::tr("Login");
        wordsToList(QStringLiteral("login"));
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case Action::NordVPN::CheckStatus: {
        title = QObject::tr("Check status");
        wordsToList(QStringLiteral("status"));
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case Action::NordVPN::Connect: {
        title = QObject::tr("Connect");
        wordsToList(QStringLiteral("c"));
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case Action::NordVPN::Disconnect: {
        title = QObject::tr("Disonnect");
        wordsToList(QStringLiteral("disconnect"));
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::Settings: {
        title = QObject::tr("Show used settings");
        wordsToList(QStringLiteral("settings"));
        forceShow = true;
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::Account: {
        title = QObject::tr("Account details");
        wordsToList(QStringLiteral("account"));
        forceShow = true;
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::Pause05: {
        title = QObject::tr("Pause for 5m");
        menuPlace = Action::MenuPlace::Common;
        break;
    }
    case Action::NordVPN::Pause30: {
        title = QObject::tr("Pause for 30m");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::Pause60: {
        title = QObject::tr("Pause for 1h");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::PauseCustom: {
        title = QObject::tr("Pause for ?");
        menuPlace = Action::MenuPlace::Own;
        break;
    }
    case Action::NordVPN::Rate5: {
        title = QObject::tr("Rate ★★★★★");
        wordsToList(QStringLiteral("rate 5"));
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case Action::NordVPN::Rate4: {
        title = QObject::tr("Rate ★★★★☆");
        wordsToList(QStringLiteral("rate 4"));
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case Action::NordVPN::Rate3: {
        title = QObject::tr("Rate ★★★☆☆");
        wordsToList(QStringLiteral("rate 3"));
        menuPlace = Action::MenuPlace::Own;
        forceShow = true;
        break;
    }
    case Action::NordVPN::Rate2: {
        title = QObject::tr("Rate ★★☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("rate 2"));
        forceShow = true;
        break;
    }
    case Action::NordVPN::Rate1: {
        title = QObject::tr("Rate ★☆☆☆☆");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("rate 1"));
        forceShow = true;
        break;
    }
    case Action::NordVPN::SetNotifyOff: {
        title = QObject::tr("Notify OFF");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set notify 0"));
        break;
    }
    case Action::NordVPN::SetNotifyOn: {
        title = QObject::tr("Notify ON");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set notify 1"));
        break;
    }
    case Action::NordVPN::KillSwitchOn: {
        title = QObject::tr("Kill Switch ON");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set killswitch 1"));
        break;
    }
    case Action::NordVPN::KillSwithcOff: {
        title = QObject::tr("Kill Switch OFF");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set killswitch 0"));
        break;
    }
    case Action::NordVPN::CyberSecOn: {
        title = QObject::tr("CyberSec ON");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set cybersec 1"));
        break;
    }
    case Action::NordVPN::CyberSecOff: {
        title = QObject::tr("CyberSec OFF");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set cybersec 0"));
        break;
    }
    case Action::NordVPN::ObfuscateOn: {
        title = QObject::tr("Obfuscate ON");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set obfuscate 1"));
        break;
    }
    case Action::NordVPN::ObfuscateOff: {
        title = QObject::tr("Obfuscate OFF");
        menuPlace = Action::MenuPlace::Own;
        wordsToList(QStringLiteral("set obfuscate 0"));
        break;
    }
    default:
        scope = Action::Flow::Custom;
        forceShow = true;
        break;
    }

    return createAction(scope, static_cast<int>(actionType), actId, appPath, title, args, forceShow, menuPlace,
                        CLICall::DefaultTimeoutMSecs, this);
}

Action::Ptr ActionStorage::createAction(Action::Flow scope, int type, const Action::Id &id, const QString &appPath,
                                        const QString &title, const QStringList &args, bool alwaysShowResult,
                                        Action::MenuPlace anchor, int timeout, QObject *parent)
{
    Action::Ptr action;

    switch (scope) {
    case Action::Flow::Yangl:
        action = m_yanglActions.value(static_cast<Action::Yangl>(type), {});
        break;
    case Action::Flow::NordVPN:
        action = m_nvpnActions.value(static_cast<Action::NordVPN>(type), {});
        break;
    default:
        if (!id.isNull())
            action = m_userActions.value(id, {});
        break;
    }

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

bool ActionStorage::updateActions(const QList<Action::Ptr> &actions, Action::Flow scope)
{
    const bool isBuiltin = scope == Action::Flow::NordVPN;
    return isBuiltin ? updateBuiltinActions(actions) : updateUserActions(actions);
}

bool ActionStorage::updateBuiltinActions(const QList<Action::Ptr> &actions)
{
    QSet<Action::NordVPN> savedActions;
    for (const auto &action : actions) {
        const Action::NordVPN actType = static_cast<Action::NordVPN>(action->type());
        if (m_nvpnActions.contains(actType))
            m_nvpnActions[actType] = action;
        else
            m_nvpnActions.insert(actType, action);
        savedActions.insert(actType);
    }

    for (auto it = m_nvpnActions.begin(); it != m_nvpnActions.end();) {
        if (!savedActions.contains(it.key())) {
            it = m_nvpnActions.erase(it);
        } else {
            ++it;
        }
    }

    return true;
}

bool ActionStorage::updateUserActions(const QList<Action::Ptr> &actions)
{
    QSet<Action::Id> savedActions;
    for (const auto &action : actions) {
        const Action::Id &actId = action->id();
        if (m_userActions.contains(actId)) {
            m_userActions[actId] = action;
        } else {
            m_userActions.insert(actId, action);
        }
        savedActions.insert(actId);
    }

    for (auto it = m_userActions.begin(); it != m_userActions.end();) {
        if (!savedActions.contains(it.key())) {
            it = m_userActions.erase(it);
        } else {
            ++it;
        }
    }

    return true;
}
