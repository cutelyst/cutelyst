/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TCPSSLSERVER_H
#define TCPSSLSERVER_H

#include <QtNetwork>

#ifndef QT_NO_SSL

#    include "tcpserver.h"

#    include <QSslConfiguration>

namespace Cutelyst {

class Server;
class Protocol;
class SslSocket;
class CWsgiEngine;
class TcpSslServer final : public TcpServer
{
    Q_OBJECT
public:
    explicit TcpSslServer(const QString &serverAddress, Protocol *protocol, Server *wsgi, QObject *parent = nullptr);

    virtual void incomingConnection(qintptr handle) override;

    virtual void shutdown() override;
    virtual void timeoutConnections() override;

    void setSslConfiguration(const QSslConfiguration &conf);

    void setHttp2Protocol(Protocol *protocol);

private:
    Protocol *m_http2Protocol = nullptr;
    QSslConfiguration m_sslConfiguration;
};

} // namespace Cutelyst

#endif // QT_NO_SSL

#endif // TCPSSLSERVER_H
