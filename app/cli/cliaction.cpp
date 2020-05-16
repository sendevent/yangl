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

void CLIAction::onResult(const QString &result) {}
