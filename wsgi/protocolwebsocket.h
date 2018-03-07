/*
 * Copyright (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef PROTOCOLWEBSOCKET_H
#define PROTOCOLWEBSOCKET_H

#include "protocol.h"

class QTextCodec;

namespace Cutelyst {
class Context;
}

namespace CWSGI {

class WSGI;
class ProtocolWebSocket : public Protocol
{
public:
    ProtocolWebSocket(WSGI *wsgi);
    ~ProtocolWebSocket();

    static QByteArray createWebsocketHeader(quint8 opcode, quint64 len);
    static QByteArray createWebsocketCloseReply(const QString &msg, quint16 closeCode);

    virtual void parse(Socket *sock, QIODevice *io) const override final;

    virtual ProtocolData *createData(Socket *sock) const override final;

private:
    bool send_text(Cutelyst::Context *c, Socket *sock, bool singleFrame) const;
    void send_binary(Cutelyst::Context *c, Socket *sock, bool singleFrame) const;
    void send_pong(QIODevice *io, const QByteArray data) const;
    void send_closed(Cutelyst::Context *c, Socket *sock, QIODevice *io) const;
    bool websocket_parse_header(Socket *sock, const char *buf, QIODevice *io) const;
    bool websocket_parse_size(Socket *sock, const char *buf, int websockets_max_message_size) const;
    void websocket_parse_mask(Socket *sock, char *buf, QIODevice *io) const;
    bool websocket_parse_payload(Socket *sock, char *buf, uint len, QIODevice *io) const;

    QTextCodec *m_codec;
    quint32 m_websockets_max_size;
};

}

#endif // PROTOCOLWEBSOCKET_H
