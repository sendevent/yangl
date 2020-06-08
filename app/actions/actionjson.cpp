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

#include "actionjson.h"

#include "action.h"
#include "actionstorage.h"
#include "common.h"
#include "settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>

static const struct {
    const struct {
        const QString Type { QStringLiteral("type") };
        const QString Scope { QStringLiteral("scope") };
        const QString Id { QStringLiteral("id") };
        const QString App { QStringLiteral("app") };
        const QString Title { QStringLiteral("title") };
        const QString Args { QStringLiteral("args") };
        const QString Display { QStringLiteral("forcedDisplay") };
        const QString Anchor { QStringLiteral("anchor") };
        const QString Timeout { QStringLiteral("timeout") };
    } Action {};

} Json;

ActionJson::ActionJson(ActionStorage *storage)
    : m_storage(storage)
{
}

void ActionJson::clear()
{
    m_json = {};
}

bool ActionJson::load(const QString &from)
{
    m_json = {};

    QFile in(from);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "failed opening file" << from << in.errorString();
        return false;
    }

    return load(&in);
}

bool ActionJson::load(QIODevice *in)
{
    m_json = {};

    if (!in || !in->isReadable())
        return false;

    const QByteArray &data = in->readAll();
    QJsonParseError err;
    const QJsonDocument &jDoc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        WRN << "error parsing document:" << err.errorString();
        return false;
    }

    m_json = jDoc.object();

    return true;
}

void ActionJson::save(const QString &to)
{
    QFile out(to);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "failed opening file" << to << out.errorString();
        return;
    }

    save(&out);
}

void ActionJson::save(QIODevice *out)
{
    if (!out || !out->isWritable())
        return;

    const QJsonDocument jDoc(m_json);
    const QByteArray &data = jDoc.toJson();
    if (-1 == out->write(data)) {
        WRN << "error during file write:" << out->errorString();
    }
}

void ActionJson::putAction(const Action *action)
{
    if (!action)
        return;

    const QString &collectionKey = action->groupKey();
    QJsonObject collection = m_json[collectionKey].toObject();
    collection[action->key()] = actionToJson(action);
    m_json[collectionKey] = collection;
}

void ActionJson::popAction(const Action *action)
{
    if (!action)
        return;

    const QString &collectionKey = action->groupKey();
    QJsonObject collection = m_json[collectionKey].toObject();
    collection.remove(action->key());
    m_json[collectionKey] = collection;
}

bool ActionJson::updateAction(Action *action)
{
    if (action) {
        const QJsonObject &collection = m_json[action->groupKey()].toObject();
        const QJsonObject &actionJson = collection[action->key()].toObject();
        if (!actionJson.isEmpty()) {
            if (const auto &loadedAction = actionFromJson(actionJson)) {
                action->setApp(loadedAction->app());
                action->setTitle(loadedAction->title());
                action->setArgs(loadedAction->args());
                action->setForcedShow(loadedAction->forcedShow());
                action->setAnchor(loadedAction->anchor());
                action->setTimeout(loadedAction->timeout());

                return true;
            }
        }
    }

    return false;
}

Action::Ptr ActionJson::actionFromJson(const QJsonObject &json) const
{
    if (json.isEmpty())
        return {};

    const auto type = static_cast<Action::NordVPN>(json[Json.Action.Type].toInt());
    const auto scope = static_cast<Action::Flow>(json[Json.Action.Scope].toInt());
    const Action::Id id = Action::Id(json[Json.Action.Id].toString());
    const auto app = json[Json.Action.App].toString();
    const auto title = json[Json.Action.Title].toString();
    const auto args = [&json]() {
        QStringList strList;
        for (const auto &str : json[Json.Action.Args].toArray()) {
            strList << str.toString();
        }
        return strList;
    }();
    const auto alwaysShowResult = json[Json.Action.Display].toBool();
    const auto anchor = static_cast<Action::MenuPlace>(json[Json.Action.Anchor].toInt());
    const auto timeout = json[Json.Action.Timeout].toInt() * yangl::OneSecondMs;

    return m_storage->createAction(scope, type, id, app, title, args, alwaysShowResult, anchor, timeout, m_storage);
}

QJsonObject ActionJson::actionToJson(const Action *action) const
{
    if (!action)
        return {};

    return {
        { Json.Action.Type, static_cast<int>(action->type()) },
        { Json.Action.Scope, static_cast<int>(action->scope()) },
        { Json.Action.Id, action->id().toString() },
        { Json.Action.App, action->app() },
        { Json.Action.Title, action->title() },
        { Json.Action.Args, QJsonArray::fromStringList(action->args()) },
        { Json.Action.Display, action->forcedShow() },
        { Json.Action.Anchor, static_cast<int>(action->anchor()) },
        { Json.Action.Timeout, action->timeout() },
    };
}

QVector<QString> ActionJson::builtinActionIds() const
{
    return actionsGroup(Action::GroupKeyBuiltin);
}

QVector<QString> ActionJson::customActionIds() const
{
    return actionsGroup(Action::GroupKeyCustom);
}

QVector<QString> ActionJson::actionsGroup(const QString &group) const
{
    if (group.isEmpty() || !m_json.contains(group))
        return {};

    QVector<QString> keys;
    const QJsonObject &collection = m_json[group].toObject();
    for (const QString &key : collection.keys())
        keys.append(key);
    return keys;
}

/*static*/ QString ActionJson::jsonFilePath()
{
    static QString jsonPath;
    if (jsonPath.isEmpty()) {
        jsonPath = QString("%1/actions.json").arg(SettingsManager::dirPath());
        QDir dir = QFileInfo(jsonPath).absoluteDir();
        if (!dir.exists())
            dir.mkpath(dir.absolutePath());
    }
    return jsonPath;
}

Action::Ptr ActionJson::action(Action::Flow scope, const QString &id)
{
    const QString &groupKey = scope == Action::Flow::NordVPN ? Action::GroupKeyBuiltin : Action::GroupKeyCustom;
    const QJsonObject &collection = m_json[groupKey].toObject();
    for (const auto &item : collection) {
        const QJsonObject &jsonAction = item.toObject();
        if (jsonAction[Json.Action.Id] == id)
            return actionFromJson(jsonAction);
    }

    return {};
}
