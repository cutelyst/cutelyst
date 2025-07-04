/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hello.h"

#include <Cutelyst/Server/server.h>

#include <QCoreApplication>
#include <QLoggingCategory>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(u"cutelyst.*.debug=true"_s);

    Cutelyst::Server server;

    QCoreApplication app(argc, argv);

    // Open HTTP/1.1 3000 port
    server.setHttpSocket({
        {u":3000"_s},
    });

    // Allow HTTP/1.1 upgrade to HTTP2 at 3000 port
    server.setUpgradeH2c(true);

    // HTTP2 requires a larger buffer size
    server.setBufferSize(16393);

    // Open HTTP/2 at 3001 port
    server.setHttp2Socket({
        {u":3001"_s},
    });

    // Open FastCGI 3002 port
    server.setFastcgiSocket({
        {u":3002"_s},
    });

    server.exec(new HelloWorld);
}
