/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#include <QFile>
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
    static constexpr QLatin1String ToggleGroup { QLatin1String("toggleGroup") };
    static constexpr QLatin1String ToggleOn { QLatin1String("toggleOn") };
};

ActionJson::ActionJson(ActionStorage *storage)
    : m_storage(storage)
{
}

void ActionJson::clear()
{
    m_json = {};
}

ActionJson::LoadResult ActionJson::tryLoad(const QString &from)
{
    m_json = {};

    QFile in(from);
    if (!in.open(QFile::ReadOnly | QFile::Text)) {
        const QString details = QStringLiteral("failed opening file %1 %2").arg(from, in.errorString());
        return std::unexpected(LoadError { LoadErrorCode::InvalidPath, details });
    }

    return tryLoad(&in);
}

ActionJson::LoadResult ActionJson::tryLoad(QIODevice *in)
{
    m_json = {};

    if (!in || !in->isReadable()) {
        return std::unexpected(LoadError { LoadErrorCode::InvalidDevice,
                                           QStringLiteral("Input device is null or not readable") });
    }

    const QByteArray &data = in->readAll();
    if (data.isEmpty()) {
        return std::unexpected(LoadError { LoadErrorCode::EmptyInput, QStringLiteral("No JSON to load") });
    }

    QJsonParseError err;
    const QJsonDocument &jDoc = QJsonDocument::fromJson(std::move(data), &err);
    if (err.error != QJsonParseError::NoError) {
        const QString details = QStringLiteral("error parsing document: %1").arg(err.errorString());
        return std::unexpected(LoadError { LoadErrorCode::InvalidJson, details });
    }

    if (!jDoc.isObject()) {
        return std::unexpected(LoadError { LoadErrorCode::InvalidRoot, QStringLiteral("JSON root is not an object") });
    }

    m_json = jDoc.object();

    return {};
}

ActionJson::SaveResult ActionJson::trySave(const QString &to)
{
    QFile out(to);
    if (!out.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        const QString details = QStringLiteral("failed opening file %1 %2").arg(to, out.errorString());
        return std::unexpected(SaveError { SaveErrorCode::InvalidPath, details });
    }

    return trySave(&out);
}

ActionJson::SaveResult ActionJson::trySave(QIODevice *out)
{
    if (!out || !out->isWritable()) {
        return std::unexpected(
                SaveError { SaveErrorCode::InvalidDevice, QStringLiteral("Output device is null or not writable") });
    }

    const QJsonDocument jDoc(m_json);
    const QByteArray &data = jDoc.toJson();
    if (-1 == out->write(data)) {
        const QString details = QStringLiteral("error during file write: %1").arg(out->errorString());
        return std::unexpected(SaveError { SaveErrorCode::WriteFailed, details });
    }

    return {};
}

void ActionJson::putAction(const Action *action)
{
    if (!action) {
        return;
    }

    const QString &collectionKey = action->groupKey();
    QJsonObject collection = m_json[collectionKey].toObject();
    collection[action->key()] = actionToJson(action);
    m_json[collectionKey] = collection;
}

void ActionJson::popAction(const Action *action)
{
    if (!action) {
        return;
    }

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
                if (!loadedAction->toggleGroup().isEmpty()) {
                    action->setToggleGroup(loadedAction->toggleGroup(), loadedAction->isToggleOn());
                }

                return true;
            }
        }
    }

    return false;
}

Action::Ptr ActionJson::actionFromJson(const QJsonObject &json) const
{
    if (json.isEmpty()) {
        return {};
    }

    const auto scope = static_cast<Action::Flow>(json[JsonAction::Scope].toInt());
    const auto type = json[JsonAction::Type].toInt();
    const Action::Id &id = Action::Id(json[JsonAction::Id].toString());
    const auto &app = json[JsonAction::App].toString();
    const auto &title = json[JsonAction::Title].toString();
    const auto &args = [&json]() {
        QStringList strList;
        const auto &array = json[JsonAction::Args].toArray();
        strList.reserve(array.size());
        std::transform(array.cbegin(), array.constEnd(), std::back_inserter(strList),
                       [](const auto &str) { return str.toString(); });
        return strList;
    }();

    const auto alwaysShowResult = json[JsonAction::Display].toBool();
    const auto anchor = static_cast<Action::MenuPlace>(json[JsonAction::Anchor].toInt());
    const auto timeout = json[JsonAction::Timeout].toInt() * utils::oneSecondMs();

    const auto &result =
            m_storage->createAction(scope, type, id, app, title, args, alwaysShowResult, anchor, timeout, m_storage);
    const auto &toggleGroup = json[JsonAction::ToggleGroup].toString();
    if (!toggleGroup.isEmpty()) {
        result->setToggleGroup(toggleGroup, json[JsonAction::ToggleOn].toBool());
    }
    return result;
}

QJsonObject ActionJson::actionToJson(const Action *action) const
{
    if (!action) {
        return {};
    }

    return {
        { JsonAction::Scope, static_cast<int>(action->scope()) },
        { JsonAction::Type, action->type() },
        { JsonAction::Id, action->id().toString() },
        { JsonAction::App, action->app() },
        { JsonAction::Title, action->title() },
        { JsonAction::Args, QJsonArray::fromStringList(action->args()) },
        { JsonAction::Display, action->forcedShow() },
        { JsonAction::Anchor, static_cast<int>(action->anchor()) },
        { JsonAction::Timeout, action->timeout() / utils::oneSecondMs() },
        { JsonAction::ToggleGroup, action->toggleGroup() },
        { JsonAction::ToggleOn, action->isToggleOn() },
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
    if (group.isEmpty() || !m_json.contains(group)) {
        return {};
    }

    QList<QString> keys;
    const auto &oldkeys = m_json[group].toObject().keys();
    std::copy(oldkeys.cbegin(), oldkeys.cend(), std::back_inserter(keys));
    return keys;
}

/*static*/ QString ActionJson::jsonFilePath()
{
    static QString jsonPath;
    if (jsonPath.isEmpty()) {
        jsonPath = utils::ensureDirExists(QString("%1/actions.json").arg(SettingsManager::dirPath()));
    }

    return jsonPath;
}

Action::Ptr ActionJson::action(Action::Flow scope, const QString &id)
{
    const QString &groupKey = Action::groupKey(scope);
    const QJsonObject &collection = m_json[groupKey].toObject();
    for (const auto &item : collection) {
        const QJsonObject &jsonAction = item.toObject();
        if (jsonAction[JsonAction::Id] == id) {
            return actionFromJson(jsonAction);
        }
    }

    return {};
}
