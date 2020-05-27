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

#include "action.h"

#include "actionresultviewer.h"
#include "actionstorage.h"
#include "clicall.h"
#include "common.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QTextBrowser>

/*static*/ const QString Action::GroupKeyBuiltin { QStringLiteral("builtin") };
/*static*/ const QString Action::GroupKeyCustom { QStringLiteral("custom") };

/*static*/ int Action::MetaIdId = -1;

Action::Action(Action::Scope scope, KnownAction type, QObject *parent, const Action::Id &id)
    : QObject(parent)
    , m_id(id.isNull() ? QUuid::createUuid() : id)
    , m_scope(scope)
    , m_type(type)
    , m_title()
    , m_app()
    , m_args()
    , m_timeout(CLICall::DefaultTimeoutMSecs)
    , m_forceShow(false)
    , m_menuPlace(MenuPlace::NoMenu)
{
    if (-1 == MetaIdId)
        MetaIdId = qRegisterMetaType<Action::Id>("Action::Id");

    connect(this, &Action::titleChanged, this, &Action::changed);
    connect(this, &Action::appChanged, this, &Action::changed);
    connect(this, &Action::argsChanged, this, &Action::changed);
    connect(this, &Action::timeoutChanged, this, &Action::changed);
    connect(this, &Action::forcedShowChanged, this, &Action::changed);
    connect(this, &Action::anchorChanged, this, &Action::changed);

    ActionResultViewer::registerAction(this);
}
Action::~Action()
{
    ActionResultViewer::unregisterAction(this);
}
Action::Scope Action::scope() const
{
    return m_scope;
}

KnownAction Action::type() const
{
    return m_type;
}

Action::Id Action::id() const
{
    return m_id;
}

QString Action::title() const
{
    return m_title;
}

void Action::setTitle(const QString &title)
{
    if (title != m_title) {
        m_title = title;
        emit titleChanged(m_title);
    }
}

QString Action::app() const
{
    return m_app;
}

void Action::setApp(const QString &app)
{
    if (app != m_app) {
        m_app = app;
        emit appChanged(m_app);
    }
}

QStringList Action::args() const
{
    return m_args;
}

void Action::setArgs(const QStringList &args)
{
    if (args != m_args) {
        m_args = args;
        emit argsChanged(m_args);
    }
}

int Action::timeout() const
{
    return m_timeout;
}

void Action::setTimeout(int timeout)
{
    if (timeout != m_timeout) {
        m_timeout = timeout;
        emit timeoutChanged(m_timeout);
    }
}

bool Action::forcedShow() const
{
    return m_forceShow;
}

void Action::setForcedShow(bool forced)
{
    if (forced != m_forceShow) {
        m_forceShow = forced;
        emit forcedShowChanged(m_forceShow);
    }
}

CLICall *Action::createRequest()
{
    if (!isValidAppPath(app()))
        return {};

    auto call = new CLICall(app(), args(), timeout(), this);
    this->QObject::connect(call, &CLICall::ready, this, &Action::onResult);

    return call;
}

/*static*/ bool Action::isValidAppPath(const QString &path)
{
    if (path.isEmpty())
        return false;

    const QFileInfo info(path);
    if (!info.exists())
        return false;

    return info.isExecutable();
}

void Action::onResult(const QString &result)
{
    int exitCode(0);
    QString errors;

    bool hasErrors(false);
    if (auto call = qobject_cast<CLICall *>(sender())) {
        exitCode = call->exitCode();
        const QString &errReport = call->errors();
        errors = errReport.isEmpty() ? errors : errReport;
        hasErrors = call->exitCode() != 0 || call->exitStatus() != QProcess::NormalExit || !errReport.isEmpty();
        call->deleteLater();
    }

    QString info = QString("%1 %2 %3:<br>").arg(yangl::now(), app(), args().join(QChar(' ')));
    if (!result.isEmpty())
        info.append(QString("<b>Result:</b><br>%1<br>").arg(QString(result).replace("\n", "<br>")));
    if (exitCode)
        info.append(QString("<b>Exit code:</b> %1<br>").arg(exitCode));
    if (!errors.isEmpty())
        info.append(QString("<b>Errors:</b> %1<br>").arg(errors));

    emit performed(m_id, result, !hasErrors, info);
}

bool Action::isAnchorable() const
{
    return anchor() != MenuPlace::NoMenu;
}

Action::MenuPlace Action::anchor() const
{
    return m_menuPlace;
}

void Action::setAnchor(MenuPlace place)
{
    if (place != anchor()) {
        m_menuPlace = place;
        emit anchorChanged(m_menuPlace);
    }
}

QString Action::groupKey() const
{
    return scope() == Action::Scope::Builtin ? GroupKeyBuiltin : GroupKeyCustom;
}

QString Action::key() const
{
    return /*scope() == Action::Scope::Builtin ? QString::number(type()) :*/ id().toString();
}
