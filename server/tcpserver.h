/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

namespace Cutelyst {

class Server;
class Protocol;
class TcpSocket;
class CWsgiEngine;
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(const QString &serverAddress, Protocol *protocol,  Server *wsgi, QObject *parent = nullptr);

    Q_INVOKABLE
    virtual void incomingConnection(qintptr handle) override;

    virtual void shutdown();
    virtual void timeoutConnections();

    Protocol *protocol() const;
    void setProtocol(Protocol *protocol);

Q_SIGNALS:
    void createConnection(qintptr handle);

protected:
    friend class TcpServerBalancer;

    QString m_serverAddress;
    CWsgiEngine *m_engine;
    Server *m_wsgi;

    std::vector<std::pair<QAbstractSocket::SocketOption, QVariant> > m_socketOptions;
    Protocol *m_protocol;
    int m_processing = 0;
};

}

#endif // TCPSERVER_H
