/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <QCoreApplication>

#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>

#include "server.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QStringLiteral("Cutelyst"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("cutelyst.org"));
    QCoreApplication::setApplicationName(QStringLiteral("cutelystd"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    Cutelyst::Server server;

    QObject::connect(&server, &Cutelyst::Server::errorOccured, [](const QString &error){
        qFatal("Server terminated due to error %s", qPrintable(error));});

    QCoreApplication app(argc, argv);

    QTranslator appTranslator;
    if (appTranslator.load(QLocale(), QStringLiteral("cutelystwsgi"), QStringLiteral("."), QStringLiteral(I18NDIR))) {
        QCoreApplication::installTranslator(&appTranslator);
    }

    server.parseCommandLine(app.arguments());

//    QTranslator qtTranslator;
//    qtTranslator.load(QLatin1String("qt_") % QLocale::system().name(),
//                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
//    QCoreApplication::installTranslator(&qtTranslator);

    return server.exec();
}
