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

#include "actions/action.h"
#include "actionstorage.h"
#include "app/common.h"
#include "settings/settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>

struct JsonAction {
    static constexpr QLatin1String Type { QLatin1String("type") };
    static constexpr QLatin1String Scope { QLatin1String("scope") };
    static constexpr QLatin1String Id { QLatin1String("id") };
    static constexpr QLatin1String App { QLatin1String("app") };
    static constexpr QLatin1String Title { QLatin1String("title") };
    static constexpr QLatin1String Args { QLatin1String("args") };
    static constexpr QLatin1String Display { QLatin1String("forcedDisplay") };
    static constexpr QLatin1String Anchor { QLatin1String("anchor") };
    static constexpr QLatin1String Timeout { QLatin1String("timeout") };
};

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

    const auto scope = static_cast<Action::Flow>(json[JsonAction::Scope].toInt());
    const auto type = json[JsonAction::Type].toInt();
    const Action::Id id = Action::Id(json[JsonAction::Id].toString());
    const auto app = json[JsonAction::App].toString();
    const auto title = json[JsonAction::Title].toString();
    const auto args = [&json]() {
        QStringList strList;
        const auto &array = json[JsonAction::Args].toArray();
        strList.reserve(array.size());
        std::transform(array.cbegin(), array.constEnd(), std::back_inserter(strList),
                       [](const auto &str) { return str.toString(); });
        return strList;
    }();

    const auto alwaysShowResult = json[JsonAction::Display].toBool();
    const auto anchor = static_cast<Action::MenuPlace>(json[JsonAction::Anchor].toInt());
    const auto timeout = json[JsonAction::Timeout].toInt() * yangl::OneSecondMs;

    return m_storage->createAction(scope, type, id, app, title, args, alwaysShowResult, anchor, timeout, m_storage);
}

QJsonObject ActionJson::actionToJson(const Action *action) const
{
    if (!action)
        return {};

    return {
        { JsonAction::Scope, static_cast<int>(action->scope()) },
        { JsonAction::Type, action->type() },
        { JsonAction::Id, action->id().toString() },
        { JsonAction::App, action->app() },
        { JsonAction::Title, action->title() },
        { JsonAction::Args, QJsonArray::fromStringList(action->args()) },
        { JsonAction::Display, action->forcedShow() },
        { JsonAction::Anchor, static_cast<int>(action->anchor()) },
        { JsonAction::Timeout, action->timeout() / yangl::OneSecondMs },
    };
}

QList<QString> ActionJson::yanglActionIds() const
{
    return actionsGroup(Action::groupKey(Action::Flow::Yangl));
}

QList<QString> ActionJson::builtinActionIds() const
{
    return actionsGroup(Action::groupKey(Action::Flow::NordVPN));
}

QList<QString> ActionJson::customActionIds() const
{
    return actionsGroup(Action::groupKey(Action::Flow::Custom));
}

QList<QString> ActionJson::actionsGroup(const QString &group) const
{
    if (group.isEmpty() || !m_json.contains(group))
        return {};

    QList<QString> keys;
    const auto &oldkeys = m_json[group].toObject().keys();
    std::copy(oldkeys.cbegin(), oldkeys.cend(), std::back_inserter(keys));
    return keys;
}

/*static*/ QString ActionJson::jsonFilePath()
{
    static QString jsonPath;
    if (jsonPath.isEmpty())
        jsonPath = yangl::ensureDirExists(QString("%1/actions.json").arg(SettingsManager::dirPath()));

    return jsonPath;
}

Action::Ptr ActionJson::action(Action::Flow scope, const QString &id)
{
    const QString &groupKey = Action::groupKey(scope);
    const QJsonObject &collection = m_json[groupKey].toObject();
    for (const auto &item : collection) {
        const QJsonObject &jsonAction = item.toObject();
        if (jsonAction[JsonAction::Id] == id)
            return actionFromJson(jsonAction);
    }

    return {};
}
