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

#include "action.h"

#include "actionresultviewer.h"
#include "actionstorage.h"
#include "app/common.h"
#include "cli/clicall.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QTextBrowser>

/*static*/ const QString Action::GroupKeyYangl { QStringLiteral("yangl") };
/*static*/ const QString Action::GroupKeyBuiltin { QStringLiteral("builtin") };
/*static*/ const QString Action::GroupKeyCustom { QStringLiteral("custom") };

/*static*/ int Action::MetaIdId = -1;

uint qHash(Action::Yangl key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

uint qHash(Action::NordVPN key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

uint qHash(Action::Flow key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

Action::Action(Action::Flow scope, int type, QObject *parent, const Action::Id &id)
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
    if (-1 == MetaIdId) {
        MetaIdId = qRegisterMetaType<Action::Id>("Action::Id");
    }

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
Action::Flow Action::scope() const
{
    return m_scope;
}

int Action::type() const
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
    if (!isValidAppPath(app())) {
        return {};
    }

    auto call = new CLICall(app(), args(), timeout(), this);
    this->QObject::connect(call, &CLICall::ready, this, &Action::onResult);
    this->QObject::connect(call, &CLICall::starting, this, &Action::onStart);

    return call;
}

/*static*/ bool Action::isValidAppPath(const QString &path)
{
    if (path.isEmpty()) {
        WRN << "Emtpy path!";
        return false;
    }

    const QFileInfo info(path);
    if (!info.exists()) {
        WRN << "file not exists!" << path;
        return false;
    }

    if (!info.isExecutable()) {
        WRN << "file is not executable!" << path;
        return false;
    }

    return true;
}

void Action::onStart(const QString &app, const QStringList &args)
{
    emit performing(id(), app, args);
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

    QString info = QString("%1 ").arg(YANGL_TIMESTAMP);
    if (!result.isEmpty()) {
        info.append(QString("<b>Result:</b><br>%1<br>").arg(QString(result).replace("\n", "<br>")));
    }
    if (exitCode) {
        info.append(QString("<b>Exit code:</b> %1<br>").arg(exitCode));
    }
    if (!errors.isEmpty()) {
        info.append(QString("<b>Errors:</b> %1<br>").arg(errors));
    }

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

/*static*/ QString Action::groupKey(Action::Flow flow)
{
    switch (flow) {
    case Action::Flow::Yangl: {
        return GroupKeyYangl;
    }
    case Action::Flow::NordVPN: {
        return GroupKeyBuiltin;
    }
    default: {
        return GroupKeyCustom;
    }
    }
}

QString Action::groupKey() const
{
    return groupKey(scope());
}

QString Action::key() const
{
    return /*scope() == Action::Scope::Builtin ? QString::number(type()) :*/ id().toString();
}

/*static*/ QList<Action::Yangl> Action::yanglActions()
{
    static QList<Action::Yangl> actions;
    if (actions.isEmpty()) {
        actions = utils::allEnum<Action::Yangl>();
    }
    return actions;
}

/*static*/ QList<Action::NordVPN> Action::nvpnActions()
{
    static QList<Action::NordVPN> actions;
    if (actions.isEmpty()) {
        actions = utils::allEnum<Action::NordVPN>({ Action::NordVPN::Unknown });
    }
    return actions;
}
