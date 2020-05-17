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

#pragma once

#include "actiontypes.h"

#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QUuid>

class QTextBrowser;
class CLICall;
class Action : public QObject
{
    Q_OBJECT
public:
    using Id = QUuid;
    using Ptr = QSharedPointer<Action>;

    enum ActScope
    {
        Builtin = 0,
        User,
    };

    enum MenuPlace
    {
        NoMenu = 0,
        Common,
        Own
    };

    virtual ~Action() = default;
    Id id() const;

    virtual Action::ActScope actionScope() const;
    virtual KnownAction type() const;

    QString title() const;
    void setTitle(const QString &title);

    QString app() const;
    void setApp(const QString &app);

    QStringList args() const;
    void setArgs(const QStringList &args);

    int timeout() const;
    void setTimeout(int timeout);

    bool forcedShow() const;
    void setForcedShow(bool forced);

    CLICall *createRequest();

    static bool isValidAppPath(const QString &path);

    bool isAnchorable() const;
    void setAnchor(MenuPlace place);
    MenuPlace menuPlace() const;

signals:
    void performed(const QString &result, bool ok);

protected slots:
    virtual void onResult(const QString &result);

protected:
    explicit Action(Action::ActScope scope, KnownAction type, QObject *parent = nullptr);
    friend class ActionStorage;

    const Action::Id m_id;
    const Action::ActScope m_scope;
    const KnownAction m_type;

    QString m_title;
    QString m_app;
    QStringList m_args;
    int m_timeout;
    bool m_forceShow;
    QPointer<QTextBrowser> m_display;
    MenuPlace m_menuPlace;
};
