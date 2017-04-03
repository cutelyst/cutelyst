/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef TCPSSLSERVER_H
#define TCPSSLSERVER_H

#include "tcpserver.h"

#include <QSslConfiguration>

namespace CWSGI {

class WSGI;
class Protocol;
class SslSocket;
class CWsgiEngine;
class TcpSslServer : public TcpServer
{
    Q_OBJECT
public:
    explicit TcpSslServer(const QString &serverAddress, Protocol *protocol,  WSGI *wsgi, QObject *parent = nullptr);

    virtual void incomingConnection(qintptr handle) override;

    virtual void shutdown() override;
    virtual void timeoutConnections() override;

    void setSslConfiguration(const QSslConfiguration &conf);

private:
    QSslConfiguration m_sslConfiguration;
};

}

#endif // TCPSSLSERVER_H
