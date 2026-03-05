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

#include "cli/clicall.h"

#include <QObject>

/*!
 * \brief Asynchronous subprocess dispatcher.
 *
 * Accepts a \c CLICall and runs it on a Qt thread-pool worker so the GUI
 * thread is never blocked. Results are delivered back to the main thread
 * via the signals on the originating \c Action.
 *
 * All NordVPN CLI interactions and user-defined script invocations flow
 * through this single choke point, making it straightforward to add
 * logging, throttling, or request queuing in one place.
 */
class CLICaller : public QObject
{
    Q_OBJECT
public:
    explicit CLICaller(QObject *parent = {});

    bool runCall(CLICall *call);

signals:
};
