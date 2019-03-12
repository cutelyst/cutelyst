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
#ifndef PROTOCOLHTTP_H
#define PROTOCOLHTTP_H

#include <QObject>

#include "protocol.h"
#include "socket.h"

#include <Cutelyst/Context>

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

    ProtoRequestHttp(Socket *sock, int bufferSize);
    ~ProtoRequestHttp() override;

    virtual void setupNewConnection(Socket *sock) override final;

    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    virtual qint64 doWrite(const char *data, qint64 len) override final;
    inline qint64 doWrite(const QByteArray &data) {
        return doWrite(data.constData(), data.size());
    }

    virtual void processingFinished() override final;

    virtual bool webSocketSendTextMessage(const QString &message) override final;

    virtual bool webSocketSendBinaryMessage(const QByteArray &message) override final;

    virtual bool webSocketSendPing(const QByteArray &payload) override final;

    virtual bool webSocketClose(quint16 code, const QString &reason) override final;

    inline virtual void resetData() override final {
        ProtocolData::resetData();

        // EngineRequest
        if (status & EngineRequest::Async) {
            context->deleteLater();
        } else {
            delete context;
        }
        context = nullptr;
        body = nullptr;

        elapsed.invalidate();
        status = InitialState;

        websocketUpgraded = false;
        last = 0;
        beginLine = 0;

        serverAddress = sock->serverAddress;
        remoteAddress = sock->remoteAddress;
        remotePort = sock->remotePort;
        isSecure = sock->isSecure;
    }

    virtual void socketDisconnected() override final;

    QByteArray websocket_message;
    QByteArray websocket_payload;
    quint64 websocket_payload_size = 0;
    quint32 websocket_need = 0;
    quint32 websocket_mask = 0;
    int last = 0;
    int beginLine = 0;
    int websocket_start_of_frame = 0;
    int websocket_phase = 0;
    quint8 websocket_continue_opcode = 0;
    quint8 websocket_finn_opcode = 0;
    bool websocketUpgraded = false;

protected:
    virtual bool webSocketHandshakeDo(const QString &key, const QString &origin, const QString &protocol) override final;
};

class ProtocolHttp2;
class ProtocolWebSocket;
class ProtocolHttp : public Protocol
{
public:
    ProtocolHttp(WSGI *wsgi, ProtocolHttp2 *upgradeH2c = nullptr);
    ~ProtocolHttp() override;

    virtual Type type() const override;

    virtual void parse(Socket *sock, QIODevice *io) const override final;

    virtual ProtocolData *createData(Socket *sock) const override final;

private:
    inline bool processRequest(Socket *sock, QIODevice *io) const;
    inline void parseMethod(const char *ptr, const char *end, Socket *sock) const;
    inline void parseHeader(const char *ptr, const char *end, Socket *sock) const;

protected:
    friend class ProtoRequestHttp;

    ProtocolWebSocket *m_websocketProto;
    ProtocolHttp2 *m_upgradeH2c;
    bool usingFrontendProxy;
};

}

#endif // PROTOCOLHTTP_H
