/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
    explicit TcpServer(const QString &serverAddress, Protocol *protocol, Server *wsgi, QObject *parent = nullptr);

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

    std::vector<std::pair<QAbstractSocket::SocketOption, QVariant>> m_socketOptions;
    Protocol *m_protocol;
    int m_processing = 0;
};

} // namespace Cutelyst

#endif // TCPSERVER_H
