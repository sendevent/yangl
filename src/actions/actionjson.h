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

#include <QJsonObject>

class QIODevice;
class ActionStorage;
class ActionJson
{
public:
    ActionJson(ActionStorage *storage);

    void clear();

    bool load(const QString &from);
    bool load(QIODevice *in);
    void save(const QString &to);
    void save(QIODevice *out);

    void putAction(const Action *action);
    void popAction(const Action *action);
    bool updateAction(Action *action);

    QVector<QString> yanglActionIds() const;
    QVector<QString> builtinActionIds() const;
    QVector<QString> customActionIds() const;

    static QString jsonFilePath();

    Action::Ptr action(Action::Flow scope, const QString &id);

private:
    QJsonObject m_json;
    ActionStorage *m_storage;

    QVector<QString> actionsGroup(const QString &group) const;
    QJsonObject actionToJson(const Action *action) const;
    Action::Ptr actionFromJson(const QJsonObject &json) const;
};
