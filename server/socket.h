/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SOCKET_H
#define SOCKET_H

#include "Cutelyst/enginerequest.h"
#include "cwsgiengine.h"
#include "protocol.h"

#include <Cutelyst/Headers>

#include <QHostAddress>
#include <QLocalSocket>
#include <QSslSocket>
#include <QTcpSocket>

class QIODevice;

namespace Cutelyst {

class Engine;
class Socket
{
    Q_GADGET
public:
    Socket(bool secure, Engine *_engine);
    virtual ~Socket();

    virtual void connectionClose() = 0;

    // Returns false if disconnected
    virtual bool requestFinished() = 0;
    virtual bool flush()           = 0;

    inline void resetSocket()
    {
        if (protoData->upgradedFrom) {
            ProtocolData *data = protoData->upgradedFrom;
            delete protoData;
            protoData = data;
        }
        processing = 0;

        protoData->resetData();
    }

    QString serverAddress;
    QHostAddress remoteAddress;
    quint16 remotePort = 0;
    Engine *engine;
    Protocol *proto         = nullptr;
    ProtocolData *protoData = nullptr;
    qint8 processing        = 0;
    bool isSecure;
    bool timeout = false;
};

class TcpSocket final : public QTcpSocket
    , public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    void connectionClose() override final;
    bool requestFinished() override final;
    bool flush() override final;
    void socketDisconnected();

Q_SIGNALS:
    // Always connect this in queued mode as the client might have
    // triggered the disconnect event like websocket close, and deleting
    // it's context from this event will crash
    void finished();
};

#ifndef QT_NO_SSL

class SslSocket final : public QSslSocket
    , public Socket
{
    Q_OBJECT
public:
    explicit SslSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    void connectionClose() override final;
    bool requestFinished() override final;
    bool flush() override final;
    void socketDisconnected();

Q_SIGNALS:
    // See TcpSocket note
    void finished();
};

#endif // QT_NO_SSL

class LocalSocket final : public QLocalSocket
    , public Socket
{
    Q_OBJECT
public:
    explicit LocalSocket(Cutelyst::Engine *engine, QObject *parent = nullptr);

    void connectionClose() override final;
    bool requestFinished() override final;
    bool flush() override final;
    void socketDisconnected();

Q_SIGNALS:
    // See TcpSocket note
    void finished();
};

} // namespace Cutelyst

#endif // SOCKET_H
