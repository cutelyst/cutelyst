/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PROTOCOLHTTP_H
#define PROTOCOLHTTP_H

#include "protocol.h"
#include "socket.h"

#include <Cutelyst/Context>

#include <QObject>

namespace Cutelyst {
class Server;
class Socket;
class ProtoRequestHttp final : public ProtocolData
    , public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    enum WebSocketPhase {
        WebSocketPhaseHeaders,
        WebSocketPhaseSize,
        WebSocketPhaseMask,
        WebSocketPhasePayload,
    };
    Q_ENUM(WebSocketPhase)

    enum OpCode {
        OpCodeContinue  = 0x0,
        OpCodeText      = 0x1,
        OpCodeBinary    = 0x2,
        OpCodeReserved3 = 0x3,
        OpCodeReserved4 = 0x4,
        OpCodeReserved5 = 0x5,
        OpCodeReserved6 = 0x6,
        OpCodeReserved7 = 0x7,
        OpCodeClose     = 0x8,
        OpCodePing      = 0x9,
        OpCodePong      = 0xA,
        OpCodeReservedB = 0xB,
        OpCodeReservedC = 0xC,
        OpCodeReservedD = 0xD,
        OpCodeReservedE = 0xE,
        OpCodeReservedF = 0xF
    };
    Q_ENUM(OpCode)

    ProtoRequestHttp(Socket *sock, int bufferSize);
    ~ProtoRequestHttp() override;

    void setupNewConnection(Socket *sock) override final;

    bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    qint64 doWrite(const char *data, qint64 len) override final;
    inline qint64 doWrite(const QByteArray &data)
    {
        return doWrite(data.constData(), data.size());
    }

    void processingFinished() override final;

    bool webSocketSendTextMessage(const QString &message) override final;

    bool webSocketSendBinaryMessage(const QByteArray &message) override final;

    bool webSocketSendPing(const QByteArray &payload) override final;

    bool webSocketClose(quint16 code, const QString &reason) override final;

    inline void resetData() override final
    {
        ProtocolData::resetData();

        // EngineRequest

        // If we deleteLater the context, there might
        // be an event that tries to finalize the request
        // and it will encounter a null context pointer
        delete context;
        context = nullptr;
        body    = nullptr;

        elapsed.invalidate();
        status = InitialState;

        websocketUpgraded = false;
        last              = 0;
        beginLine         = 0;

        serverAddress = sock->serverAddress;
        remoteAddress = sock->remoteAddress;
        remotePort    = sock->remotePort;
        isSecure      = sock->isSecure;
    }

    virtual void socketDisconnected() override final;

    QByteArray websocket_message;
    QByteArray websocket_payload;
    quint64 websocket_payload_size   = 0;
    quint32 websocket_need           = 0;
    quint32 websocket_mask           = 0;
    int last                         = 0;
    int beginLine                    = 0;
    int websocket_start_of_frame     = 0;
    int websocket_phase              = 0;
    quint8 websocket_continue_opcode = 0;
    quint8 websocket_finn_opcode     = 0;
    bool websocketUpgraded           = false;

protected:
    bool webSocketHandshakeDo(const QString &key, const QString &origin, const QString &protocol) override final;
};

class ProtocolHttp2;
class ProtocolWebSocket;
class ProtocolHttp final : public Protocol
{
public:
    ProtocolHttp(Server *wsgi, ProtocolHttp2 *upgradeH2c = nullptr);
    ~ProtocolHttp() override;

    Type type() const override;

    void parse(Socket *sock, QIODevice *io) const override final;

    ProtocolData *createData(Socket *sock) const override final;

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

} // namespace Cutelyst

#endif // PROTOCOLHTTP_H
