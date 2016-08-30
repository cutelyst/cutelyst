/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <QCoreApplication>
#include <QCommandLineParser>

#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>

#include <iostream>

#include "wsgi.h"
#include "config.h"
//#include <eventdispatcher_epoll.h>

int main(int argc, char *argv[])
{
    //    QCoreApplication::setEventDispatcher(new EventDispatcherEPoll);

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Cutelyst"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("cutelyst.org"));
    QCoreApplication::setApplicationName(QStringLiteral("cutelyst-wsgi"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    QTranslator qtTranslator;
    qtTranslator.load(QLatin1String("qt_") % QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Fast, developer-friendly WSGI server"));
    parser.addHelpOption();
    parser.addVersionOption();

    auto ini = QCommandLineOption(QStringLiteral("ini"),
                                  QStringLiteral("load config from ini file"),
                                  QStringLiteral("file"));
    parser.addOption(ini);

    auto chdir = QCommandLineOption(QStringLiteral("chdir"),
                                    QStringLiteral("chdir to specified directory before apps loading"),
                                    QStringLiteral("directory"));
    parser.addOption(chdir);

    auto chdir2 = QCommandLineOption(QStringLiteral("chdir2"),
                                     QStringLiteral("chdir to specified directory afterapps loading"),
                                     QStringLiteral("directory"));
    parser.addOption(chdir2);

    auto application = QCommandLineOption({ QStringLiteral("application"), QStringLiteral("a") },
                                          QStringLiteral("Application to load"),
                                          QStringLiteral("file"));
    parser.addOption(application);

    auto threads = QCommandLineOption({ QStringLiteral("threads"), QStringLiteral("t") },
                                      QStringLiteral("Number of thread to use"),
                                      QStringLiteral("threads"));
    parser.addOption(threads);

#ifdef Q_OS_UNIX
    auto process = QCommandLineOption({ QStringLiteral("process"), QStringLiteral("p") },
                                      QStringLiteral("spawn the specified number of processes"),
                                      QStringLiteral("processes"));
    parser.addOption(process);
#endif // Q_OS_UNIX

    auto master = QCommandLineOption({ QStringLiteral("master"), QStringLiteral("M") },
                                      QStringLiteral("Enable master process"));
    parser.addOption(master);

    auto bufferSize = QCommandLineOption({ QStringLiteral("buffer-size"), QStringLiteral("b") },
                                         QStringLiteral("set internal buffer size"),
                                         QStringLiteral("bytes"));
    parser.addOption(bufferSize);

    auto httpSocket = QCommandLineOption({ QStringLiteral("http-socket"), QStringLiteral("h1") },
                                         QStringLiteral("bind to the specified TCP socket using HTTP protocol"),
                                         QStringLiteral("address"));
    parser.addOption(httpSocket);

    QCommandLineOption restart = QCommandLineOption({ QStringLiteral("restart"), QStringLiteral("r") },
                                                    QStringLiteral("Restarts when the application file changes"));
    parser.addOption(restart);

    // Process the actual command line arguments given by the user
    parser.process(app);


    CWSGI::WSGI wsgi;
    if (parser.isSet(ini)) {
        wsgi.setIni(parser.value(ini));
    }

    if (parser.isSet(chdir)) {
        wsgi.setChdir(parser.value(chdir));
    }

    if (parser.isSet(chdir2)) {
        wsgi.setChdir(parser.value(chdir2));
    }

    if (parser.isSet(threads)) {
        wsgi.setThreads(parser.value(threads).toInt());
    }

#ifdef Q_OS_UNIX
    if (parser.isSet(process)) {
        wsgi.setProcess(parser.value(process).toInt());
    }
#endif // Q_OS_UNIX

    if (parser.isSet(application)) {
        wsgi.setApplication(parser.value(application));
    }

    bool masterSet = parser.isSet(master);
    wsgi.setMaster(masterSet);

    if (!masterSet && parser.isSet(httpSocket)) {
        const auto socks = parser.values(httpSocket);
        for (const QString &http : socks) {
            wsgi.setHttpSocket(http);
        }
    }

    if (wsgi.application().isEmpty()) {
        std::cout << "Application is not defined" << std::endl;
        parser.showHelp(1);
    }

    if (!wsgi.loadApplication()) {
        return 1;
    }

    return app.exec();
}
