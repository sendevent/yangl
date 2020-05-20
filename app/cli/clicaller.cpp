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

#include "clicaller.h"

#include "action.h"

#include <QDebug>
#include <QRunnable>
#include <QThreadPool>

#define LOG qDebug() << Q_FUNC_INFO << QThread::currentThreadId()

class RunCallTask : public QRunnable
{
public:
    RunCallTask(CLICall *call)
        : m_call(call)
    {
    }

private:
    CLICall *m_call;

    void run() override { m_call->run(); }
};

CLICaller::CLICaller(QObject *parent)
    : QObject(parent)
{
}

bool CLICaller::performAction(Action *action)
{
    if (!action)
        return false;

    if (auto call = action->createRequest()) {
        runQuery(call);
        return true;
    }

    return false;
}

void CLICaller::runQuery(CLICall *call)
{
    if (!call)
        return;

    RunCallTask *task = new RunCallTask(call);
    QThreadPool::globalInstance()->start(task);
}
