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
#include "common.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>

/*static*/ constexpr int CLICall::DefaultTimeoutMSecs;

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
    if (m_appPath.isEmpty() || !QFile::exists(m_appPath))
        return setResult({}, tr("File [%1] not found").arg(m_appPath));

    QProcess proc;

    connect(
            &proc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                m_exitCode = exitCode;
                m_exitStatus = exitStatus;
            },
            Qt::DirectConnection);

    emit starting(m_appPath, m_params);

    proc.start(m_appPath, m_params, QIODevice::ReadOnly);

    if (!proc.waitForStarted(m_timeout))
        return setResult({}, tr("Start timeout (%1) reached for [%2]").arg(QString::number(m_timeout), m_appPath));

    auto stripSpinner = [](QString &in) {
        static const QString spinnerString("-\\|/ \r");
        while (!in.isEmpty() && spinnerString.contains(in.at(0)))
            in.remove(0, 1);
        return in;
    };

    QString result, errors;
    while (proc.waitForReadyRead(m_timeout)) {
        QString in(proc.readAllStandardOutput());
        result += stripSpinner(in);
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
    if (errors != m_errors)
        m_errors = errors;

    if (result != m_result)
        m_result = result;

    emit ready(m_result);

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

bool CLICall::success() const
{
    return exitCode() == 0 && exitStatus() == QProcess::NormalExit;
}
