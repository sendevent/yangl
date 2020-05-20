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
#include "settingsmanager.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#define LOG qDebug() << Q_FUNC_INFO
#define WRN qWarning() << Q_FUNC_INFO

static const struct {
    const QString BuiltinCollection { QStringLiteral("builtin") };
    const QString CustomCollection { QStringLiteral("custom") };

    const struct {
        const QString App { QStringLiteral("app") };
        const QString Title { QStringLiteral("title") };
        const QString Args { QStringLiteral("args") };
        const QString Display { QStringLiteral("forcedDisplay") };
        const QString Anchor { QStringLiteral("anchor") };
    } Action {};

} Json;

ActionJson::ActionJson() {}

/*static*/ QString ActionJson::filePath()
{
    static const QString jsonPath = QString("%1/actions.json").arg(SettingsManager::dirPath());
    return jsonPath;
}

bool ActionJson::load()
{
    static const QString jsonFilePath(filePath());
    m_json = {};

    QFile in(jsonFilePath);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "failed opening file" << jsonFilePath << in.errorString();
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

void ActionJson::save()
{
    static const QString jsonFilePath(filePath());

    QFile out(jsonFilePath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "failed opening file" << jsonFilePath << out.errorString();
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

    const QString &collectionKey = this->collectionKey(action);
    QJsonObject collection = m_json[collectionKey].toObject();
    collection[actionKey(action)] = actionToJson(action);
    m_json[collectionKey] = collection;
}

void ActionJson::popAction(const Action *action)
{
    if (!action)
        return;

    const QString &collectionKey = this->collectionKey(action);
    QJsonObject collection = m_json[collectionKey].toObject();
    collection.remove(actionKey(action));
    m_json[collectionKey] = collection;
}

bool ActionJson::updateAction(Action *action)
{
    if (!action)
        return false;

    const QString &collectionKey = this->collectionKey(action);
    const QJsonObject &collection = m_json[collectionKey].toObject();
    const QJsonObject &actionJson = collection[actionKey(action)].toObject();
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

    return true;
}

QString ActionJson::collectionKey(const Action *action) const
{
    QString key;
    if (action) {
        const bool isBuiltint = action->actionScope() == Action::ActScope::Builtin;
        key = isBuiltint ? Json.BuiltinCollection : Json.CustomCollection;
    }
    return key;
}

QString ActionJson::actionKey(const Action *action) const
{
    QString key;
    if (action) {
        const bool isBuiltint = action->actionScope() == Action::ActScope::Builtin;
        key = isBuiltint ? QString::number(action->type()) : action->id().toString();
    }
    return key;
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
        { Json.Action.Anchor, static_cast<const int>(action->anchor()) },
    };
}

QVector<QString> ActionJson::customActionIds() const
{
    QVector<QString> keys;
    const QJsonObject &collection = m_json[Json.CustomCollection].toObject();
    for (const QString &key : collection.keys())
        keys.append(key);
    return keys;
}
