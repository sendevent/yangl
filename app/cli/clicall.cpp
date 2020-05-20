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

#include "clicall.h"

#include "clicaller.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#define LOG qDebug() << now() << Q_FUNC_INFO << QThread::currentThreadId()

static QString now()
{
    QDateTime d = QDateTime::currentDateTime();
    return d.toString("hh:mm:ss.zzz");
}

CLICall::CLICall(const QString &path, const QStringList &params, int timeout, QObject *parent)
    : QObject(parent)
    , m_appPath(path)
    , m_params(params)
    , m_timeout(timeout)
    , m_result()
    , m_errors()
    , m_exitCode(0)
    , m_exitStatus(QProcess::NormalExit)
{
}

QString CLICall::run()
{
    LOG << m_appPath << m_params;
    if (m_appPath.isEmpty() || !QFile::exists(m_appPath))
        return setResult({}, QStringLiteral("File [%1] not found").arg(m_appPath));

    QProcess proc;

    connect(
            &proc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                m_exitCode = exitCode;
                m_exitStatus = exitStatus;
            },
            Qt::DirectConnection);

    proc.start(m_appPath, m_params, QIODevice::ReadOnly);

    if (!proc.waitForStarted(m_timeout))
        return setResult(
                {}, QStringLiteral("Start timeout (%1) reached for [%2]").arg(QString::number(m_timeout), m_appPath));

    QString result, errors;
    while (proc.waitForReadyRead(m_timeout)) {
        result += proc.readAllStandardOutput();
        errors += proc.readAllStandardError();
    }

    return setResult(result.trimmed(), errors.trimmed());
}

QString CLICall::result() const
{
    return m_result;
}

QString CLICall::setResult(const QString &result, const QString &errors)
{
    LOG << result << errors << exitCode() << exitStatus();
    if (errors != m_errors)
        m_errors = errors;

    if (result != m_result) {
        m_result = result;
        emit ready(m_result);
    }

    return m_result;
}

QString CLICall::errors() const
{
    return m_errors;
}

int CLICall::exitCode() const
{
    return m_exitCode;
}

QProcess::ExitStatus CLICall::exitStatus() const
{
    return m_exitStatus;
}
