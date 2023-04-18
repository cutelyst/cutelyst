/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hello.h"

#include <server/server.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    Cutelyst::Server server;

    QCoreApplication app(argc, argv);

    // Open HTTP/1.1 3000 port
    server.setHttpSocket({
        {QStringLiteral(":3000")},
    });

    // Allow HTTP/1.1 upgrade to HTTP2 at 3000 port
    server.setUpgradeH2c(true);

    // HTTP2 requires a larger buffer size
    server.setBufferSize(16393);

    // Open HTTP/2 at 3001 port
    server.setHttp2Socket({
        {QStringLiteral(":3001")},
    });

    // Open FastCGI 3002 port
    server.setFastcgiSocket({
        {QStringLiteral(":3002")},
    });

    server.exec(new HelloWorld);
}
