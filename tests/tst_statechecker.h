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

#include "actionstorage.h"
#include "clicaller.h"
#include "statechecker.h"

#include <QObject>
#include <QSharedPointer>

class tst_StateChecker : public QObject
{
    Q_OBJECT
public:
    explicit tst_StateChecker(QObject *parent = nullptr);

private slots:
    void init();

    void test_active();
    void test_interval();
    void test_check_status_change();

private:
    const std::unique_ptr<CLICaller> m_caller;
    const std::unique_ptr<ActionStorage> m_storage;
    QSharedPointer<StateChecker> m_checker;
    void test_check(NordVpnInfo::Status targetStatus);
};
