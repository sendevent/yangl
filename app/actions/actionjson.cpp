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

#include "actionjson.h"

#include "action.h"
#include "common.h"
#include "settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>

static const struct {
    const struct {
        const QString App { QStringLiteral("app") };
        const QString Title { QStringLiteral("title") };
        const QString Args { QStringLiteral("args") };
        const QString Display { QStringLiteral("forcedDisplay") };
        const QString Anchor { QStringLiteral("anchor") };
        const QString Timeout { QStringLiteral("timeout") };
    } Action {};

} Json;

ActionJson::ActionJson() {}

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
    if (!action)
        return false;

    const QString &collectionKey = action->groupKey();
    const QJsonObject &collection = m_json[collectionKey].toObject();
    const QJsonObject &actionJson = collection[action->key()].toObject();
    if (actionJson.isEmpty())
        return false;

    action->setApp(actionJson[Json.Action.App].toString());
    action->setTitle(actionJson[Json.Action.Title].toString());
    action->setArgs([&actionJson]() {
        QStringList strList;
        for (const auto &str : actionJson[Json.Action.Args].toArray()) {
            strList << str.toString();
        }
        return strList;
    }());
    action->setForcedShow(actionJson[Json.Action.Display].toBool());
    action->setAnchor(static_cast<Action::MenuPlace>(actionJson[Json.Action.Anchor].toInt()));
    action->setTimeout(actionJson[Json.Action.Timeout].toInt() * yangl::OneSecondMs);

    return true;
}

QJsonObject ActionJson::actionToJson(const Action *action) const
{
    if (!action)
        return {};

    return {
        { Json.Action.App, action->app() },
        { Json.Action.Title, action->title() },
        { Json.Action.Args, QJsonArray::fromStringList(action->args()) },
        { Json.Action.Display, action->forcedShow() },
        { Json.Action.Anchor, static_cast<int>(action->anchor()) },
        { Json.Action.Timeout, action->timeout() },
    };
}

QVector<QString> ActionJson::customActionIds() const
{
    QVector<QString> keys;
    const QJsonObject &collection = m_json[Action::GroupKeyCustom].toObject();
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
