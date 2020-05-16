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

#include "clicall.h"

#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QUuid>

class QTextBrowser;
class CLIAction : public QObject
{
    Q_OBJECT
public:
    using Id = QUuid;
    using Ptr = QSharedPointer<CLIAction>;

    enum Scope
    {
        Builtin = 0,
        User
    };

    explicit CLIAction(const CLIAction::Scope scope, QObject *parent = nullptr);
    Id id() const;

    CLIAction::Scope scope() const;

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

signals:
    void performed(const QString &result, bool ok);

private slots:
    void onResult(const QString &result);

private:
    const CLIAction::Id m_id;
    const CLIAction::Scope m_scope;
    QString m_title;
    QString m_app;
    QStringList m_args;
    int m_timeout;
    bool m_forceShow;
    QPointer<QTextBrowser> m_display;
};
