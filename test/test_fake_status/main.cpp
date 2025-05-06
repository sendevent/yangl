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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <iostream>
#include <ostream>

void printLine(const QString &line)
{
    std::cout << line.toStdString() << std::endl;
}

void printConnecting()
{
    printLine("Status: Connecting");
    printLine("Current server: fi88.nordvpn.com");
    printLine("Country: Finland");
    printLine("City: Helsinki");
    printLine("Your new IP: 196.196.203.67");
    printLine("Current technology: OpenVPN");
    printLine("Current protocol: UDP");
}

void printConnected()
{
    printLine("Status: Connected");
    printLine("Current server: fi88.nordvpn.com");
    printLine("Country: Finland");
    printLine("City: Helsinki");
    printLine("Your new IP: 196.196.203.67");
    printLine("Current technology: OpenVPN");
    printLine("Current protocol: UDP");
    printLine("Transfer: 0.97 MiB received, 452.22 KiB sent");
    printLine("Uptime: 3 hours 24 minutes 5 seconds");
}

void printDisconnected()
{
    printLine("Status: Disconnected");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(
            "YANGL Test helper: the CLI tool to fake expected NVPN statuses and check their handling");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption optionConnecting { { "i", "status-connecting" },
                                                QCoreApplication::translate("main", "Emulate \"connecting\" output") };
    const QCommandLineOption optionConnected { { "e", "status-connected" },
                                               QCoreApplication::translate("main", "Emulate \"connected\" output") };
    const QCommandLineOption optionDisconnected {
        { "d", "status-disconnected" }, QCoreApplication::translate("main", "Emulate \"disconnected\" output")
    };

    parser.addOptions({
            optionConnecting,
            optionConnected,
            optionDisconnected,
    });

    parser.process(a);

    if (parser.optionNames().isEmpty()) {
        printLine(parser.helpText());
        return 10;
    }

    if (parser.isSet(optionConnecting)) {
        printConnecting();
    } else if (parser.isSet(optionConnected)) {
        printConnected();
    } else if (parser.isSet(optionDisconnected)) {
        printDisconnected();
    } else {
        return 20;
    }

    return 0;
}
