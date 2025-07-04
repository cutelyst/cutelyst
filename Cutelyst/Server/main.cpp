/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "config.h"
#include "server.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(u"Cutelyst"_s);
    QCoreApplication::setOrganizationDomain(u"cutelyst.org"_s);
    QCoreApplication::setApplicationName(u"cutelystd"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(CUTELYST_VERSION));

    Cutelyst::Server server;

    QObject::connect(&server, &Cutelyst::Server::errorOccured, [](const QString &error) {
        qFatal("Server terminated due to error %s", qPrintable(error));
    });

    QCoreApplication app(argc, argv);

    QTranslator appTranslator;
    if (appTranslator.load(
            QLocale(), u"cutelystserver"_s, u"."_s, QStringLiteral(CUTELYST_I18N_DIR))) {
        QCoreApplication::installTranslator(&appTranslator);
    }

    server.parseCommandLine(QCoreApplication::arguments());

    //    QTranslator qtTranslator;
    //    qtTranslator.load(u"qt_" % QLocale::system().name(),
    //                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    //    QCoreApplication::installTranslator(&qtTranslator);

    return server.exec();
}
