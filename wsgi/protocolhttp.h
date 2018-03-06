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
#ifndef PROTOCOLHTTP_H
#define PROTOCOLHTTP_H

#include <QObject>

#include "protocol.h"
#include "socket.h"

namespace CWSGI {

class WSGI;
class Socket;

class ProtoRequestHttp : public ProtocolData, public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    enum WebSocketPhase
    {
        WebSocketPhaseHeaders,
        WebSocketPhaseSize,
        WebSocketPhaseMask,
        WebSocketPhasePayload,
    };
    Q_ENUM(WebSocketPhase)

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

    ProtoRequestHttp(WSGI *wsgi, Cutelyst::Engine *_engine);
    virtual ~ProtoRequestHttp();

    virtual qint64 doWrite(const char *data, qint64 len) override final;
    inline qint64 doWrite(const QByteArray &data) {
        return doWrite(data.constData(), data.size());
    }

    virtual bool webSocketSendTextMessage(const QString &message) override final;

    virtual bool webSocketSendBinaryMessage(const QByteArray &message) override final;

    virtual bool webSocketSendPing(const QByteArray &payload) override final;

    virtual bool webSocketClose(quint16 code, const QString &reason) override final;

    Cutelyst::Context *websocketContext = nullptr;
    QByteArray websocket_message;
    QByteArray websocket_payload;
    quint32 websocket_need;
    quint32 websocket_mask;
    int websocket_start_of_frame = 0;
    int websocket_phase = 0;
    int websocket_payload_size;
    quint8 websocket_continue_opcode = 0;
    quint8 websocket_finn_opcode;

protected:
    virtual bool webSocketHandshakeDo(Cutelyst::Context *c, const QString &key, const QString &origin, const QString &protocol) /*override final*/;
};

class ProtocolWebSocket;
class ProtocolHttp : public Protocol
{
public:
    ProtocolHttp(WSGI *wsgi);
    ~ProtocolHttp();

    virtual Type type() const override;

    virtual void readyRead(Socket *sock, QIODevice *io) const override;
    virtual bool sendHeaders(QIODevice *io, CWSGI::Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers) override;

private:
    inline bool processRequest(Socket *sock) const;
    inline void parseMethod(const char *ptr, const char *end, Socket *sock) const;
    inline void parseHeader(const char *ptr, const char *end, Socket *sock) const;

    ProtocolWebSocket *m_websocketProto;
};

}

#endif // PROTOCOLHTTP_H
