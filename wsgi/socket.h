/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QSslSocket>
#include <QLocalSocket>
#include <QHostAddress>
#include <Cutelyst/Headers>
#include <Cutelyst/engineconnection.h>

#include "cwsgiengine.h"

class QIODevice;

namespace CWSGI {

class WSGI;

class Protocol;
class Socket : public Cutelyst::EngineConnection
{
    Q_GADGET
public:
    Socket(WSGI *wsgi, Cutelyst::Engine *_engine);
    virtual ~Socket();

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
        ContentBody
    };
    Q_ENUM(ParserState)

    enum OpCode
    {
        OpCodeContinue    = 0x0,
        OpCodeText        = 0x1,
        OpCodeBinary      = 0x2,
        OpCodeReserved3   = 0x3,
        OpCodeReserved4   = 0x4,
        OpCodeReserved5   = 0x5,
        OpCodeReserved6   = 0x6,
        OpCodeReserved7   = 0x7,
        OpCodeClose       = 0x8,
        OpCodePing        = 0x9,
        OpCodePong        = 0xA,
        OpCodeReservedB   = 0xB,
        OpCodeReservedC   = 0xC,
        OpCodeReservedD   = 0xD,
        OpCodeReservedE   = 0xE,
        OpCodeReservedF   = 0xF
    };
    Q_ENUM(OpCode)

    enum WebSocketPhase
    {
        WebSocketPhaseHeaders,
        WebSocketPhaseSize,
        WebSocketPhaseMask,
        WebSocketPhasePayload,
    };
    Q_ENUM(WebSocketPhase)

    inline void resetSocket() {
        connState = MethodLine;
        stream_id = 0; //FCGI
        buf_size = 0;
        beginLine = 0;
        last = 0;
        startOfRequest = 0;
        headerConnection = HeaderConnectionNotSet;
        pktsize = 0;
        processing = false;
        headerHost = false;
        timeout = false;
        delete body;
        body = nullptr;
        status = InitialState;
    }

    virtual void connectionClose() = 0;

    qint64 contentLength;
    QIODevice *io;
    Cutelyst::Context *websocketContext = nullptr;
    Protocol *proto;
    char *buffer;
    ParserState connState = MethodLine;
    quint64 stream_id = 0;// FGCI
    quint32 buf_size = 0;
    quint32 last = 0;
    int beginLine = 0;
    HeaderConnection headerConnection = HeaderConnectionNotSet;
    quint16 pktsize = 0;// FGCI
    bool headerHost = false;
    bool processing = false;
    bool timeout = false;

    QByteArray websocket_message;
    QByteArray websocket_payload;
    quint32 websocket_need;
    int websocket_start_of_frame = 0;
    int websocket_phase = 0;
    int websocket_payload_size;
    quint32 websocket_mask;
    quint8 websocket_continue_opcode = 0;
    quint8 websocket_finn_opcode;

    virtual bool webSocketSendTextMessage(Cutelyst::Context *c, const QString &message) final;

    virtual bool webSocketSendBinaryMessage(Cutelyst::Context *c, const QByteArray &message) final;

    virtual bool webSocketSendPing(Cutelyst::Context *c, const QByteArray &payload) final;

    virtual bool webSocketClose(Cutelyst::Context *c, quint16 code, const QString &reason) final;

protected:
    virtual qint64 doWrite(const char *data, qint64 len) final;

    inline qint64 doWrite(const QByteArray &data) {
        return doWrite(data.constData(), data.size());
    }

    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) final;

    virtual bool webSocketHandshakeDo(Cutelyst::Context *c, const QString &key, const QString &origin, const QString &protocol) final;
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
