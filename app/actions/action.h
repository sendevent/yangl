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
class ActionStorage;
class Action : public QObject
{
    Q_OBJECT
public:
    using Id = QUuid;

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    class Ptr : public QSharedPointer<Action>
    {
    public:
        Ptr(Action *action = {})
            : QSharedPointer<Action>(action)
        {
        }

        Action *get() const { return data(); }
    };
#else
    using Ptr = QSharedPointer<Action>;
#endif

    enum class Scope
    {
        Builtin = 0,
        User,
    };

    enum class MenuPlace
    {
        NoMenu = 0,
        Common,
        Own
    };

    virtual ~Action();
    Id id() const;

    virtual Action::Scope scope() const;
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
    Action::MenuPlace anchor() const;

    static const QString GroupKeyBuiltin;
    static const QString GroupKeyCustom;

    QString groupKey() const;
    QString key() const;

signals:
    void performed(const Action::Id &id, const QString &result, bool ok, const QString &description);
    void changed();

    void titleChanged(const QString &title) const;
    void appChanged(const QString &app) const;
    void argsChanged(const QStringList &args) const;
    void timeoutChanged(int timeout) const;
    void forcedShowChanged(bool forced) const;
    void anchorChanged(Action::MenuPlace place) const;

protected slots:
    virtual void onResult(const QString &result);

protected:
    friend class ActionStorage;
    static int MetaIdId;

    explicit Action(Action::Scope scope, KnownAction type, QObject *parent = {}, const Action::Id &id = {});

    const Action::Id m_id;
    const Action::Scope m_scope;
    const KnownAction m_type;

    QString m_title;
    QString m_app;
    QStringList m_args;
    int m_timeout;
    bool m_forceShow;
    QPointer<QTextBrowser> m_display;
    MenuPlace m_menuPlace;
};

Q_DECLARE_METATYPE(Action::MenuPlace);
