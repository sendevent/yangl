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

#include <QtTest>

// add necessary includes here

class dummy : public QObject
{
    Q_OBJECT

public:
    dummy();
    ~dummy();

private slots:
    void test_case1();

};

dummy::dummy()
{

}

dummy::~dummy()
{

}

void dummy::test_case1()
{

}

QTEST_APPLESS_MAIN(dummy)

#include "tst_dummy.moc"
