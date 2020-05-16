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

#include "cliaction.h"

#include "clicall.h"

#include <QApplication>
#include <QFileInfo>
#include <QTextBrowser>

CLIAction::CLIAction(const CLIAction::Scope scope, QObject *parent)
    : QObject(parent)
    , m_scope(scope)
    , m_title()
    , m_app()
    , m_args()
    , m_timeout(CLICall::DefaultTimeoutMSecs)
    , m_forceShow(false)
{
}

CLIAction::Scope CLIAction::scope() const
{
    return m_scope;
}

QString CLIAction::title() const
{
    return m_title;
}

void CLIAction::setTitle(const QString &title)
{
    if (title != m_title)
        m_title = title;
}

QString CLIAction::app() const
{
    return m_app;
}

void CLIAction::setApp(const QString &app)
{
    if (app != m_app)
        m_app = app;
}

QStringList CLIAction::args() const
{
    return m_args;
}

void CLIAction::setArgs(const QStringList &args)
{
    if (args != m_args)
        m_args = args;
}

int CLIAction::timeout() const
{
    return m_timeout;
}

void CLIAction::setTimeout(int timeout)
{
    if (timeout != m_timeout)
        m_timeout = timeout;
}

bool CLIAction::forcedShow() const
{
    return m_forceShow;
}

void CLIAction::setForcedShow(bool forced)
{
    if (forced != m_forceShow)
        m_forceShow = forced;
}

CLICall::Ptr CLIAction::createRequest()
{
    if (!isValidAppPath(app()))
        return {};

    CLICall::Ptr call(new CLICall(app(), args(), timeout()));
    connect(call.get(), &CLICall::ready, this, &CLIAction::onResult);
    return call;
}

/*static*/ bool CLIAction::isValidAppPath(const QString &path)
{
    if (path.isEmpty())
        return false;

    const QFileInfo info(path);
    if (!info.exists())
        return false;

    return info.isExecutable();
}

void CLIAction::onResult(const QString &result)
{
    QString exitCode(tr("Unknown"));
    QString errors(tr("No errors"));

    bool hasErrors(false);
    if (auto call = qobject_cast<CLICall *>(sender())) {
        exitCode = call->exitCode();
        const QString &errReport = call->errors();
        errors = errReport.isEmpty() ? errors : errReport;
        hasErrors = call->exitCode() != 0 || call->exitStatus() != QProcess::NormalExit || !errReport.isEmpty();
    }

    if (hasErrors || forcedShow()) {
        QTextBrowser *display = new QTextBrowser;
        display->setAttribute(Qt::WA_DeleteOnClose);
        display->setWindowTitle(QStringLiteral("%1 — %2").arg(qApp->applicationDisplayName(), title()));
        display->setReadOnly(true);
        display->append(tr("Call to \"%1\" [%2]<br>"
                           "Result<br>"
                           "%3<br>"
                           "Exit code: %4<br>"
                           "Errors:<br>"
                           "%5")
                                .arg(app(), args().join(" "), result, errors));
        display->show();
    }

    emit performed(result, !hasErrors);
}
