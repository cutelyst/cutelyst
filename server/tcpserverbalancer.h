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
class CWsgiEngine;
class Protocol;
class TcpServerBalancer final : public QTcpServer
{
    Q_OBJECT
public:
    TcpServerBalancer(Server *parent);
    ~TcpServerBalancer() override;

    bool listen(const QString &address, Protocol *protocol, bool secure);

    void setBalancer(bool enable);
    QString serverName() const { return m_serverName; }

    void incomingConnection(qintptr handle) override;

    TcpServer *createServer(CWsgiEngine *engine);

private:
    QHostAddress m_address;
    quint16 m_port = 0;
    QString m_serverName;
    std::vector<TcpServer *> m_servers;
    Server *m_wsgi;
    Protocol *m_protocol                  = nullptr;
    QSslConfiguration *m_sslConfiguration = nullptr;
    int m_currentServer                   = 0;
    bool m_balancer                       = false;
};

} // namespace Cutelyst

#endif // TCPSERVERBALANCER_H
