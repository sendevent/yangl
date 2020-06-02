/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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

#include "tst_action.h"
#include "tst_actionjson.h"
#include "tst_actionstorage.h"
#include "tst_clicall.h"
#include "tst_clicaller.h"
#include "tst_statechecker.h"

#include <QApplication>
#include <QtTest>

int main(int argc, char **argv)
{
    QApplication app(argc, argv); // init standard paths

    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject *obj) {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };

    ASSERT_TEST(new tst_Action());
    ASSERT_TEST(new tst_ActionJson());
    ASSERT_TEST(new tst_CLICall());
    ASSERT_TEST(new tst_CLICaller());
    ASSERT_TEST(new tst_ActionStorage());
    ASSERT_TEST(new tst_StateChecker());

    return status;
}
