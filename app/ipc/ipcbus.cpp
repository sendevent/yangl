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

#include "ipcbus.h"

#include <QDebug>
#include <QRunnable>
#include <QThreadPool>

#define LOG qDebug() << Q_FUNC_INFO << QThread::currentThreadId()

class RunQureyTask : public QRunnable
{
public:
    RunQureyTask(const IPCCall::Ptr &call)
        : m_call(call)
    {
    }

private:
    IPCCall::Ptr m_call;

    void run() override { m_call->run(); }
};

IPCBus::IPCBus(const QString &appPath, QObject *parent)
    : QObject(parent)
    , m_appPath(appPath)
{
}

QString IPCBus::applicationPath() const
{
    return m_appPath;
}

void IPCBus::runQuery(const IPCCall::Ptr &call)
{
    if (!call)
        return;

    RunQureyTask *task = new RunQureyTask(call);
    QThreadPool::globalInstance()->start(task);
}
