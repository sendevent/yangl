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

void printConnecting()
{
    std::cout << "Status: Connecting\n \
                 Current server: fi88.nordvpn.com\n \
                 Country: Finland\n \
                 City: Helsinki\n \
                 Your new IP: 196.196.203.67\n \
                 Current technology: OpenVPN\n \
                 Current protocol: UDP";
}

void printConnected()
{
    std::cout << "Status: Connected\n \
                 Current server: fi88.nordvpn.com\n \
                 Country: Finland\n \
                 City: Helsinki\n \
                 Your new IP: 196.196.203.67\n \
                 Current technology: OpenVPN\n \
                 Current protocol: UDP\n \
                 Transfer: 0.97 MiB received, 452.22 KiB sent\n \
                 Uptime: 3 hours 24 minutes 5 seconds";
}

void printDisconnected()
{
    std::cout << "Status: Disconnected\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("YANGL Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    auto addOption = [&parser](const QString &name, const QString &description) {
        QCommandLineOption option(name, description);
        parser.addOption(option);
        return option;
    };

    const QCommandLineOption &optionConnecting =
            addOption("status-connecting", QCoreApplication::translate("main", "Emulate \"connecting\" output"));
    const QCommandLineOption &optionConnected =
            addOption("status-connected", QCoreApplication::translate("main", "Emulate \"connected\" output"));
    const QCommandLineOption &optionDisconnected =
            addOption("status-disconnected", QCoreApplication::translate("main", "Emulate \"disconnected\" output"));

    parser.process(a);

    if (parser.isSet(optionConnecting)) {
        printConnecting();
        return 0;
    }

    if (parser.isSet(optionConnected)) {
        printConnected();
        return 0;
    }

    if (parser.isSet(optionDisconnected)) {
        printDisconnected();
        return 0;
    }

    return 1;
}
