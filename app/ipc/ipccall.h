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
#include <QSharedPointer>
#include <QStringList>
#include <QUuid>

class IPCCall : public QObject
{
    Q_OBJECT

public:
    using Id = QUuid;
    using Ptr = QSharedPointer<IPCCall>;

    explicit IPCCall(const QString &path, const QStringList &params, int timeout);
    virtual ~IPCCall() = default;

    Id id() const;

    QString run();
    QString result() const;

signals:
    void ready(const QString &result) const;

protected:
    const Id m_id;
    const QString m_appPath;
    const QStringList m_params;
    const int m_timeout;
    QString m_result;

    QString setResult(const QString &result);

private:
    Q_DISABLE_COPY_MOVE(IPCCall);
};
