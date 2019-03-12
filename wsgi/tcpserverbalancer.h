/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef TCPSERVERBALANCER_H
#define TCPSERVERBALANCER_H

#include <QTcpServer>
#include <QtGlobal>

class QSslConfiguration;

namespace CWSGI {

class WSGI;
class TcpServer;
class CWsgiEngine;
class Protocol;
class TcpServerBalancer : public QTcpServer
{
    Q_OBJECT
public:
    TcpServerBalancer(WSGI *parent);
    ~TcpServerBalancer() override;

    bool listen(const QString &address, Protocol *protocol, bool secure);

    void setBalancer(bool enable);
    QString serverName() const { return m_serverName; }

    virtual void incomingConnection(qintptr handle) override;

    TcpServer *createServer(CWsgiEngine *engine);

private:
    QHostAddress m_address;
    quint16 m_port = 0;
    QString m_serverName;
    std::vector<TcpServer *> m_servers;
    WSGI *m_wsgi;
    Protocol *m_protocol = nullptr;
    QSslConfiguration *m_sslConfiguration = nullptr;
    int m_currentServer = 0;
    bool m_balancer = false;
};

}

#endif // TCPSERVERBALANCER_H
