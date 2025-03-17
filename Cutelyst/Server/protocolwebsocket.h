/*
 * SPDX-FileCopyrightText: (C) 2017-2019 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PROTOCOLWEBSOCKET_H
#define PROTOCOLWEBSOCKET_H

#include "protocol.h"

namespace Cutelyst {
class Context;
class Server;
class ProtocolWebSocket final : public Protocol
{
public:
    explicit ProtocolWebSocket(Server *wsgi);
    virtual ~ProtocolWebSocket() override;

    Type type() const override;

    static QByteArray createWebsocketHeader(quint8 opcode, quint64 len);
    static QByteArray createWebsocketCloseReply(const QString &msg, quint16 closeCode);

    void parse(Socket *sock, QIODevice *io) const override final;

    ProtocolData *createData(Socket *sock) const override final;

private:
    bool send_text(Cutelyst::Context *c, Socket *sock, bool singleFrame) const;
    void send_binary(Cutelyst::Context *c, Socket *sock, bool singleFrame) const;
    void send_pong(QIODevice *io, const QByteArray &data) const;
    void send_closed(const Context *c, Socket *sock, QIODevice *io) const;
    bool websocket_parse_header(Socket *sock, const char *buf, QIODevice *io) const;
    bool websocket_parse_size(Socket *sock, const char *buf, int websockets_max_message_size) const;
    void websocket_parse_mask(Socket *sock, char *buf, QIODevice *io) const;
    bool websocket_parse_payload(Socket *sock, char *buf, int len, QIODevice *io) const;

    int m_websockets_max_size;
};

} // namespace Cutelyst

#endif // PROTOCOLWEBSOCKET_H
