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

#pragma once

#include <QObject>
#include <QProcess>
#include <QSharedPointer>
#include <QStringList>

class CLICall : public QObject
{
    Q_OBJECT

public:
    static constexpr int DefaultTimeoutMSecs = 30000;

    explicit CLICall(const QString &path, const QStringList &params, int timeout, QObject *parent = {});
    ~CLICall() = default;

    QString run();
    QString result() const;
    QString errors() const;

    int exitCode() const;
    QProcess::ExitStatus exitStatus() const;
    bool success() const;

signals:
    void ready(const QString &result) const;

protected:
    const QString m_appPath;
    const QStringList m_params;
    const int m_timeout;
    QString m_result, m_errors;
    int m_exitCode;
    QProcess::ExitStatus m_exitStatus;

    QString setResult(const QString &result, const QString &errors);

private:
    CLICall(QObject *parent = {}) = delete;

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    Q_DISABLE_COPY_MOVE(CLICall);
#endif
};
