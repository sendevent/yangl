/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <chrono>

class CLICall : public QObject
{
    Q_OBJECT

public:
    using Timeout = std::chrono::milliseconds;
    static constexpr Timeout DefaultTimeout = std::chrono::seconds(30);
    static constexpr int DefaultTimeoutMSecs = static_cast<int>(DefaultTimeout.count());

    explicit CLICall(const QString &path, const QStringList &params, Timeout timeout, QObject *parent = {});
    explicit CLICall(const QString &path, const QStringList &params, int timeout, QObject *parent = {});
    ~CLICall() = default;

    QString run();
    QString result() const;
    QString errors() const;

    int exitCode() const;
    QProcess::ExitStatus exitStatus() const;
    bool success() const;

signals:
    void starting(const QString &myApp, const QStringList &myArgs);
    void ready(const QString &result);

protected:
    const QString m_appPath;
    const QStringList m_params;
    const Timeout m_timeout;
    QString m_result, m_errors;
    int m_exitCode { 0 };
    QProcess::ExitStatus m_exitStatus { QProcess::NormalExit };

    QString setResult(const QString &result, const QString &errors);

private:
    CLICall(QObject *parent = {}) = delete;

    Q_DISABLE_COPY_MOVE(CLICall);
};
