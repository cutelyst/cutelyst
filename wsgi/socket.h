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
#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QSslSocket>
#include <QLocalSocket>
#include <QHostAddress>
#include <Cutelyst/Headers>
#include "Cutelyst/enginerequest.h"

#include "cwsgiengine.h"

#include "hpack.h"

class QIODevice;

namespace Cutelyst {
class Engine;
}

namespace CWSGI {

struct H2Stream
{
    enum state {
        Idle,
        Open,
        HalfClosed,
        Closed
    };

    QString method;
    QString path;
    QString scheme;
    QString authority;
    Cutelyst::Headers headers;
    quint32 streamId;
    quint32 windowSize = 65535;
    qint64 contentLength = -1;
    qint64 consumedData = 0;
    quint8 state = Idle;
};

class WSGI;
class Socket;
class Protocol;
class ProtocolData /* : public Cutelyst::EngineRequest*/
{
    Q_GADGET
public:
    ProtocolData();
    virtual ~ProtocolData();

    enum HeaderConnection {
        HeaderConnectionNotSet = 0,
        HeaderConnectionKeep,
        HeaderConnectionClose,
        HeaderConnectionUpgrade
    };
    Q_ENUM(HeaderConnection)

    enum ParserState {
        MethodLine = 0,
        HeaderLine,
        ContentBody,
        H2Frames
    };
    Q_ENUM(ParserState)

    inline void resetSocket() {
        connState = MethodLine;
//        stream_id = 0; //FCGI
        buf_size = 0;
        beginLine = 0;
        last = 0;
//        startOfRequest = 0;
//        maxStreamId = 0;
//        streamForContinuation = 0;
//        windowSize = 65535;
        headerConnection = HeaderConnectionNotSet;
        pktsize = 0;
//        processing = false;
        headerHost = false;
//        canPush = false;
//        delete body;
//        body = nullptr;
//        status = InitialState;
//        delete hpack;
//        hpack = nullptr;
//        qDeleteAll(streams);
//        streams.clear();
    }

    Socket *sock;//temporary
    qint64 contentLength;
    QIODevice *io;
    char *buffer;
    ParserState connState = MethodLine;
    quint32 buf_size = 0;
    quint32 last = 0;
    int beginLine = 0;
    HeaderConnection headerConnection = HeaderConnectionNotSet;
    quint32 pktsize = 0;// FGCI-16 H2-24
    bool headerHost = false;


protected:
//    virtual qint64 doWrite(const char *data, qint64 len)/* override final*/;

//    inline qint64 doWrite(const QByteArray &data) {
//        return doWrite(data.constData(), data.size());
//    }

//    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers)/* override final*/;
};

class Socket
{
    Q_GADGET
public:
    Socket(WSGI *wsgi, Cutelyst::Engine *_engine);
    virtual ~Socket();

    virtual void connectionClose() = 0;

    inline void resetSocket() {

    }

    Cutelyst::Engine *engine;
    Protocol *proto;
    ProtocolData *protoData;
    bool timeout = false;
    bool processing = false;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(TcpSocket *bj);
};

class SslSocket : public QSslSocket, public Socket
{
    Q_OBJECT
public:
    explicit SslSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(SslSocket *bj);
};

class LocalSocket : public QLocalSocket, public Socket
{
    Q_OBJECT
public:
    explicit LocalSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent = nullptr);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(LocalSocket *bj);
};

}

#endif // SOCKET_H
