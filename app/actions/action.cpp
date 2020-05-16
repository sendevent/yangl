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

#include "clicall.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QTextBrowser>

#define LOG qDebug() << Q_FUNC_INFO

Action::Action(const Action::Scope scope, QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid())
    , m_scope(scope)
    , m_title()
    , m_app()
    , m_args()
    , m_timeout(CLICall::DefaultTimeoutMSecs)
    , m_forceShow(false)
{
}

Action::Id Action::id() const
{
    return m_id;
}

Action::Scope Action::scope() const
{
    return m_scope;
}

QString Action::title() const
{
    return m_title;
}

void Action::setTitle(const QString &title)
{
    if (title != m_title)
        m_title = title;
}

QString Action::app() const
{
    return m_app;
}

void Action::setApp(const QString &app)
{
    if (app != m_app)
        m_app = app;
}

QStringList Action::args() const
{
    return m_args;
}

void Action::setArgs(const QStringList &args)
{
    if (args != m_args)
        m_args = args;
}

int Action::timeout() const
{
    return m_timeout;
}

void Action::setTimeout(int timeout)
{
    if (timeout != m_timeout)
        m_timeout = timeout;
}

bool Action::forcedShow() const
{
    return m_forceShow;
}

void Action::setForcedShow(bool forced)
{
    if (forced != m_forceShow)
        m_forceShow = forced;
}

CLICall *Action::createRequest()
{
    if (!isValidAppPath(app()))
        return {};

    auto call = new CLICall(app(), args(), timeout(), this);
    connect(call, &CLICall::ready, this, &Action::onResult);

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
    QString exitCode(tr("Unknown"));
    QString errors(tr("No errors"));

    bool hasErrors(false);
    if (auto call = qobject_cast<CLICall *>(sender())) {
        exitCode = QString::number(call->exitCode(), 16);
        const QString &errReport = call->errors();
        errors = errReport.isEmpty() ? errors : errReport;
        hasErrors = call->exitCode() != 0 || call->exitStatus() != QProcess::NormalExit || !errReport.isEmpty();
        call->deleteLater();
    }

    if (hasErrors || forcedShow()) {
        if (!m_display) {
            m_display = new QTextBrowser;
            m_display->setAttribute(Qt::WA_DeleteOnClose);
            m_display->setReadOnly(true);
        }

        m_display->setWindowTitle(QStringLiteral("%1 — %2").arg(qApp->applicationDisplayName(), title()));
        m_display->append(tr("Call to <b>%1</b> [%2]<br>"
                             "<b>Result:</b><br>"
                             "%3<br>"
                             "<b>Exit code:</b> %4<br>"
                             "<b>Errors:</b><br>"
                             "%5")
                                  .arg(app(), args().join(" "), result, exitCode, errors));
        m_display->show();
    }

    emit performed(result, !hasErrors);
}
