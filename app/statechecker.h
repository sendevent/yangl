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

#include "action.h"

#include <QObject>
#include <QQueue>

class CLIBus;
class ActionStorage;
class QTimer;
class StateChecker : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Unknown = 0,
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };
    Q_ENUM(Status);

    struct Info {
        Status m_status;
        QString m_server;
        QString m_country;
        QString m_city;
        QString m_ip;
        QString m_technology;
        QString m_protocol;
        QString m_traffic;
        QString m_uptime;

        void clear();
        bool operator==(const Info &other) const;
        bool operator!=(const Info &other) const;
        QString toString() const;

        static StateChecker::Info fromString(const QString &text);
        static StateChecker::Status textToStatus(const QString &from);
        static QString statusToText(StateChecker::Status from);
        static QString parseUptime(const QString &from);
    };

    explicit StateChecker(CLIBus *bus, ActionStorage *actions, QObject *parent = nullptr);
    ~StateChecker() override;

    void check();

    bool isActive() const;
    int inteval() const;
    Info state() const;

public slots:
    void setInterval(int msecs);
    void setActive(bool active);

signals:
    void stateChanged(const StateChecker::Info &state);
    void statusChanged(const StateChecker::Status status);

private slots:
    void onTimeout();
    void onQueryFinish(const QString &result, bool ok);

protected:
    CLIBus *m_bus;
    ActionStorage *m_actions;
    QQueue<Action::Ptr> m_calls;
    Action::Ptr m_currAction;
    QTimer *m_timer;

    Info m_state;
    void setState(const Info &state);
    void setStatus(Status status);

    void nextQuery();
    void updateState(const QString &from);
};
Q_DECLARE_METATYPE(StateChecker::Status)
Q_DECLARE_METATYPE(StateChecker::Info)
