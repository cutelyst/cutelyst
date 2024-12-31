/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TCPSERVERBALANCER_H
#define TCPSERVERBALANCER_H

#include <QTcpServer>
#include <QtGlobal>

class QSslConfiguration;

namespace Cutelyst {

class Server;
class TcpServer;
class ServerEngine;
class Protocol;
class TcpServerBalancer final : public QTcpServer
{
    Q_OBJECT
public:
    TcpServerBalancer(Server *parent);
    ~TcpServerBalancer() override;

    bool listen(const QString &address, Protocol *protocol, bool secure);

    void setBalancer(bool enable);
    QByteArray serverName() const { return m_serverName; }

    void incomingConnection(qintptr handle) override;

    TcpServer *createServer(ServerEngine *engine);

private:
    QHostAddress m_address;
    quint16 m_port = 0;
    QByteArray m_serverName;
    std::vector<TcpServer *> m_servers;
    Server *m_server;
    Protocol *m_protocol                  = nullptr;
    QSslConfiguration *m_sslConfiguration = nullptr;
    int m_currentServer                   = 0;
    bool m_balancer                       = false;
};

} // namespace Cutelyst

#endif // TCPSERVERBALANCER_H
