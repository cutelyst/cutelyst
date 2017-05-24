/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "protocolwebsocket.h"

#include "socket.h"
#include "wsgi.h"

#include <Cutelyst/Headers>
#include <Cutelyst/Context>

#include <QLoggingCategory>

using namespace CWSGI;

Q_LOGGING_CATEGORY(CWSGI_WS, "cwsgi.websocket")

ProtocolWebSocket::ProtocolWebSocket(CWSGI::WSGI *wsgi) : Protocol(wsgi)
{
    m_websockets_max_size = 1024 * 1024;
    m_wsBufferSize = qMax(static_cast<qint64>(32), wsgi->postBufferingBufsize());
    m_wsBuffer = new char[m_wsBufferSize];
}

ProtocolWebSocket::~ProtocolWebSocket()
{
    delete [] m_wsBuffer;
}

QByteArray ProtocolWebSocket::createWebsocketReply(const QByteArray &msg, quint8 opcode)
{
    QByteArray ret;
    ret.append(0x80 + opcode);

    quint64 len = msg.length();
    if (len < 126) {
        ret.append(static_cast<quint8>(len));
    } else if (len <= static_cast<quint16>(0xffff)) {
        ret.append(126);

        quint8 buf[2];
        buf[1] = (quint8) (len & 0xff);
        buf[0] = (quint8) ((len >> 8) & 0xff);
        ret.append((char*) buf, 2);
    } else {
        ret.append(127);

        quint8 buf[8];
        buf[7] = (quint8) (len & 0xff);
        buf[6] = (quint8) ((len >> 8) & 0xff);
        buf[5] = (quint8) ((len >> 16) & 0xff);
        buf[4] = (quint8) ((len >> 24) & 0xff);
        buf[3] = (quint8) ((len >> 32) & 0xff);
        buf[2] = (quint8) ((len >> 40) & 0xff);
        buf[1] = (quint8) ((len >> 48) & 0xff);
        buf[0] = (quint8) ((len >> 56) & 0xff);
        ret.append((char*) buf, 8);
    }
    ret.append(msg);

    return ret;
}

quint16 ws_be16(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint16 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[1] = static_cast<quint8>(*src & 0xff);
    return ret;
}

quint64 ws_be64(const char *buf) {
    const quint64 *src = reinterpret_cast<const quint64 *>(buf);
    quint64 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 56) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 48) & 0xff);
    ptr[2] = static_cast<quint8>((*src >> 40) & 0xff);
    ptr[3] = static_cast<quint8>((*src >> 32) & 0xff);
    ptr[4] = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[5] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[6] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[7] = static_cast<quint8>(*src & 0xff);
    return ret;
}

static bool websocket_parse_header(Socket *sock, const char *buf)
{
    quint8 byte1 = buf[0];
    sock->websocket_finn_opcode = byte1;
    quint8 opcode = byte1 & 0xf;
    if (opcode == Socket::OpCodeText || opcode == Socket::OpCodeBinary) {
        sock->websocket_message = QByteArray();
        if (!(byte1 & 0x80)) {
            // FINN byte not set, store opcode for continue
            sock->websocket_continue_opcode = opcode;
        }
    }

    quint8 byte2 = buf[1];
    bool websocket_has_mask = byte2 >> 7;
    if (!websocket_has_mask) {
        // RFC states that client to server MUST have a mask
        qCCritical(CWSGI_WS) << "Client did not set MASK bit" << sock << opcode << sock->websocket_continue_opcode;
        sock->connectionClose();
        return false;
    }

    qDebug() << "HEADER" << opcode << (byte2 & 0x7f);
    sock->websocket_payload_size = byte2 & 0x7f;
    if (sock->websocket_payload_size == 126) {
        sock->websocket_need = 2;
        sock->websocket_phase = Socket::WebSocketPhaseSize;
    } else if (sock->websocket_payload_size == 127) {
        sock->websocket_need = 8;
        sock->websocket_phase = Socket::WebSocketPhaseSize;
    } else {
        sock->websocket_need = 4;
        sock->websocket_phase = Socket::WebSocketPhaseMask;
    }

    return true;
}

static bool websocket_parse_size(Socket *sock, const char *buf, int websockets_max_message_size)
{
    if (sock->websocket_payload_size == 126) {
        quint16 ret = ws_be16(buf);
        if (ret > INT_MAX) {
            qCCritical(CWSGI_WS) << "Payload size too big" << ret << "max allowed" << INT_MAX;
            sock->connectionClose();
            return false;
        }
        sock->websocket_payload_size = ret;
    } else if (sock->websocket_payload_size == 127) {
        quint64 ret = ws_be64(buf);
        if (ret > INT_MAX) {
            qCCritical(CWSGI_WS) << "Payload size too big" << ret << "max allowed" << INT_MAX;
            sock->connectionClose();
            return false;
        }
        sock->websocket_payload_size = ret;
    } else {
        qCCritical(CWSGI_WS) << "BUG error in websocket parser:" << sock->websocket_payload_size;
        sock->connectionClose();
        return false;
    }

    if (sock->websocket_payload_size > websockets_max_message_size) {
        qCCritical(CWSGI_WS) << "Invalid packet size received:" << sock->websocket_payload_size
                             << ", max allowed:" << websockets_max_message_size;
        sock->connectionClose();
        return false;
    }

    sock->websocket_need = 4;
    sock->websocket_phase = Socket::WebSocketPhaseMask;

    return true;
}

static void websockets_parse_mask(Socket *sock, const char *buf)
{
    const quint32 *ptr = reinterpret_cast<const quint32 *>(buf);
    sock->websocket_mask = *ptr;

    sock->websocket_need = sock->websocket_payload_size;
    sock->websocket_phase = Socket::WebSocketPhasePayload;

    sock->websocket_payload = QByteArray();
    sock->websocket_payload.reserve(sock->websocket_payload_size);
}

static void send_text(Cutelyst::Context *c, Socket *sock, bool singleFrame)
{
    Cutelyst::Request *request = c->request();

    const QString frame = QString::fromUtf8(sock->websocket_payload);
    request->webSocketTextFrame(frame,
                                sock->websocket_finn_opcode & 0x80,
                                sock->websocketContext);

    if (sock->websocket_finn_opcode & 0x80) {
        if (singleFrame || sock->websocket_payload == sock->websocket_message) {
            request->webSocketTextMessage(frame,
                                          sock->websocketContext);
        } else {
            request->webSocketTextMessage(QString::fromUtf8(sock->websocket_message),
                                          sock->websocketContext);
        }
    }
}

static void send_binary(Cutelyst::Context *c, Socket *sock, bool singleFrame)
{
    Cutelyst::Request *request = c->request();

    const QByteArray frame = sock->websocket_payload;
    request->webSocketBinaryFrame(frame,
                                  sock->websocket_finn_opcode & 0x80,
                                  sock->websocketContext);

    if (sock->websocket_finn_opcode & 0x80) {
        if (singleFrame || sock->websocket_payload == sock->websocket_message) {
            request->webSocketBinaryMessage(frame,
                                            sock->websocketContext);
        } else {
            request->webSocketBinaryMessage(sock->websocket_message,
                                            sock->websocketContext);
        }
    }
}

static bool websockets_parse_payload(Socket *sock, char *buf, uint len, QIODevice *io)
{
    quint8 *mask = reinterpret_cast<quint8 *>(&sock->websocket_mask);
    for (uint i = 0, maskIx = sock->websocket_payload.size(); i < len; ++i, ++maskIx) {
        buf[i] = buf[i] ^ mask[maskIx % 4];
    }

    sock->websocket_payload.append(buf, len);
    if (sock->websocket_payload.size() < sock->websocket_payload_size) {
        // need more data
        sock->websocket_need -= len;
        return true;
    }
    sock->websocket_message.append(sock->websocket_payload);

    sock->websocket_need = 2;
    sock->websocket_phase = Socket::WebSocketPhaseHeaders;

    Cutelyst::Request *request = sock->websocketContext->request();

    switch (sock->websocket_finn_opcode & 0xf) {
    case Socket::OpCodeContinue:
        switch (sock->websocket_continue_opcode) {
        case Socket::OpCodeText:
            send_text(sock->websocketContext, sock, false);
            break;
        case Socket::OpCodeBinary:
            send_binary(sock->websocketContext, sock, false);
            break;
        default:
            qCCritical(CWSGI_WS) << "Invalid CONTINUE opcode:" << (sock->websocket_finn_opcode & 0xf);
            sock->connectionClose();
            return false;
        }
        break;
    case Socket::OpCodeText:
        send_text(sock->websocketContext, sock, sock->websocket_finn_opcode & 0x80);
        break;
    case Socket::OpCodeBinary:
        send_binary(sock->websocketContext, sock, sock->websocket_finn_opcode & 0x80);
        break;
    case Socket::OpCodeClose:
        sock->connectionClose();
        return false;
    case Socket::OpCodePing:
        io->write(ProtocolWebSocket::createWebsocketReply(sock->websocket_payload.left(125), Socket::OpCodePong));
        break;
    case Socket::OpCodePong:
        request->webSocketPong(sock->websocket_payload,
                               sock->websocketContext);
        break;
    default:
        break;
    }

    return true;
}

void ProtocolWebSocket::readyRead(Socket *sock, QIODevice *io) const
{
    qint64 bytesAvailable = io->bytesAvailable();
    qCDebug(CWSGI_WS) << "ProtocolWebSocket::readyRead" << bytesAvailable;

    Q_FOREVER {
        if (!bytesAvailable || (bytesAvailable < sock->websocket_need && sock->websocket_phase != Socket::WebSocketPhasePayload)) {
            // Need more data
            return;
        }

        quint32 maxlen = qMin(sock->websocket_need, m_wsBufferSize);
        qint64 len = io->read(m_wsBuffer, maxlen);
        if (len == -1) {
            qCWarning(CWSGI_WS) << "Failed to read from socket" << io->errorString();
            sock->connectionClose();
            return;
        }
        bytesAvailable -= len;

        switch(sock->websocket_phase) {
        case Socket::WebSocketPhaseHeaders:
            if (!websocket_parse_header(sock, m_wsBuffer)) {
                return;
            }
            break;
        case Socket::WebSocketPhaseSize:
            if (!websocket_parse_size(sock, m_wsBuffer, m_websockets_max_size)) {
                return;
            }
            break;
        case Socket::WebSocketPhaseMask:
            websockets_parse_mask(sock, m_wsBuffer);
            break;
        case Socket::WebSocketPhasePayload:
            if (!websockets_parse_payload(sock, m_wsBuffer, len, io)) {
                return;
            }
            break;
        }
    }
}

bool ProtocolWebSocket::sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers)
{
    Q_UNUSED(io)
    Q_UNUSED(sock)
    Q_UNUSED(status)
    Q_UNUSED(dateHeader)
    Q_UNUSED(headers)
    qFatal("ProtocolWebSocket::sendHeaders() called!");
    return false;
}
