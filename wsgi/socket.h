/*
 * Copyright (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QSslSocket>
#include <QLocalSocket>
#include <QHostAddress>
#include <Cutelyst/Headers>
#include "Cutelyst/enginerequest.h"

#include "cwsgiengine.h"

#include "protocol.h"

class QIODevice;

namespace Cutelyst {
class Engine;
}

namespace CWSGI {

class WSGI;
class Socket
{
    Q_GADGET
public:
    Socket(bool secure, Cutelyst::Engine *_engine);
    virtual ~Socket();

    virtual void connectionClose() = 0;

    inline void resetSocket() {
        protoData->resetSocket();
        processing = false;
    }

    QString serverAddress;
    QHostAddress remoteAddress;
    quint16 remotePort;
    Cutelyst::Engine *engine;
    Protocol *proto;
    ProtocolData *protoData;
    bool isSecure;
    bool timeout = false;
    bool processing = false;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(TcpSocket *bj);
};

class SslSocket : public QSslSocket, public Socket
{
    Q_OBJECT
public:
    explicit SslSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(SslSocket *bj);
};

class LocalSocket : public QLocalSocket, public Socket
{
    Q_OBJECT
public:
    explicit LocalSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(LocalSocket *bj);
};

}

#endif // SOCKET_H
