/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    QCoreApplication::setApplicationName(QStringLiteral("cutelyst-wsgi"));
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
