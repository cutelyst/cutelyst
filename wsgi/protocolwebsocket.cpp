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
        buf[1] = (uint8_t) (len & 0xff);
        buf[0] = (uint8_t) ((len >> 8) & 0xff);
        ret.append((char*) buf, 2);
    } else {
        ret.append(127);

        quint8 buf[8];
        buf[7] = (uint8_t) (len & 0xff);
        buf[6] = (uint8_t) ((len >> 8) & 0xff);
        buf[5] = (uint8_t) ((len >> 16) & 0xff);
        buf[4] = (uint8_t) ((len >> 24) & 0xff);
        buf[3] = (uint8_t) ((len >> 32) & 0xff);
        buf[2] = (uint8_t) ((len >> 40) & 0xff);
        buf[1] = (uint8_t) ((len >> 48) & 0xff);
        buf[0] = (uint8_t) ((len >> 56) & 0xff);
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

static void websocket_parse_header(Socket *sock) {
    quint8 byte1 = sock->buffer[0];
    quint8 byte2 = sock->buffer[1];
    sock->websocket_opcode = byte1 & 0xf;
    sock->websocket_has_mask = byte2 >> 7;
    sock->websocket_size = byte2 & 0x7f;
}

static QByteArray websockets_parse(Socket *sock)
{
    // de-mask buffer
    uint8_t *ptr = (uint8_t *) (sock->buffer + (sock->websocket_pktsize - sock->websocket_size));
    size_t i;

    if (sock->websocket_has_mask) {
        uint8_t *mask = ptr-4;
        for(i=0;i<sock->websocket_size;i++) {
            ptr[i] = ptr[i] ^ mask[i%4];
        }
    }

    auto ub = QByteArray((char *) ptr, sock->websocket_size);

    sock->buf_size -= sock->websocket_pktsize;
    memmove(sock->buffer, sock->buffer + sock->websocket_pktsize, sock->buf_size);
    sock->websocket_phase = 0;
    sock->websocket_need = 2;

    return ub;
}

void ProtocolWebSocket::readyRead(Socket *sock, QIODevice *io) const
{
    qCDebug(CWSGI_WS) << "ProtocolWebSocket::readyRead";

    int len = io->read(sock->buffer + sock->buf_size, m_bufferSize - sock->buf_size);
    if (len == -1) {
        qCWarning(CWSGI_WS) << "Failed to read from socket" << io->errorString();
        sock->connectionClose();
        return;
    }
    qCDebug(CWSGI_WS) << "m_bufferSize" << m_bufferSize ;
    qCDebug(CWSGI_WS) << "len" << len ;
    qCDebug(CWSGI_WS) << "sock->buf_size" << sock->buf_size ;
    sock->buf_size += len;

    Q_FOREVER {
        quint32 remains = sock->buf_size;
        // i have data;
        //        qCDebug(CWSGI_WS) << "sock->buf_size" << sock->buf_size << remains << sock->websocket_need;
        if (remains >= sock->websocket_need) {
            qCDebug(CWSGI_WS) << "sock->websocket_phase" << sock->websocket_phase << remains << sock->websocket_need;
            switch(sock->websocket_phase) {
            // header
            case 0:
                websocket_parse_header(sock);
                sock->websocket_pktsize = 2 + (sock->websocket_has_mask * 4);
                if (sock->websocket_size == 126) {
                    sock->websocket_need += 2;
                    sock->websocket_phase = 1;
                    sock->websocket_pktsize += 2;
                }
                else if (sock->websocket_size == 127) {
                    sock->websocket_need += 8;
                    sock->websocket_phase = 1;
                    sock->websocket_pktsize += 8;
                }
                else {
                    sock->websocket_phase = 2;
                }
                qCDebug(CWSGI_WS) << 0 << sock->websocket_need << sock->websocket_phase << sock->websocket_pktsize;
                break;
                // size
            case 1:
                if (sock->websocket_size == 126) {
                    sock->websocket_size = ws_be16(sock->buffer + 2);
                }
                else if (sock->websocket_size == 127) {
                    sock->websocket_size = ws_be64(sock->buffer + 2);
                }
                else {
                    qCDebug(CWSGI_WS,  " BUG error in websocket parser");
                    return;
                }
                if (sock->websocket_size > (sock->buf_size)) {
                    qCDebug(CWSGI_WS,  " invalid packet size received");

                    //                    uwsgi_log("[uwsgi-websocket] \"%.*s %.*s\" (%.*s) invalid packet size received: %llu, max allowed: %llu\n", REQ_DATA, sock->websocket_size, uwsgi.websockets_max_size * 1024);
                    return;
                }
                sock->websocket_phase = 2;
                break;
                // mask check
            case 2:
                if (sock->websocket_has_mask) {
                    sock->websocket_need += 4;
                    sock->websocket_phase = 3;
                }
                else {
                    sock->websocket_need += sock->websocket_size;
                    sock->websocket_phase = 4;
                }
                break;
                // mask
            case 3:
                sock->websocket_pktsize += sock->websocket_size;
                sock->websocket_need += sock->websocket_size;
                sock->websocket_phase = 4;
                break;
                // message
            case 4:
                switch (sock->websocket_opcode) {
                // message
                case Socket::OpCodeContinue:
                    qDebug() << "CONTINUE" << websockets_parse(sock);
                    return;
                case Socket::OpCodeText:
                    sock->websocketContext->request()->websocketTextMessage(QString::fromUtf8(websockets_parse(sock)),
                                                                            sock->websocketContext);
                    return;
                case Socket::OpCodeBinary:
                    sock->websocketContext->request()->websocketBinaryMessage(websockets_parse(sock),
                                                                              sock->websocketContext);
                    return;
                    // close
                case Socket::OpCodeClose:
                    return ;
                    // ping
                case Socket::OpCodePing:
                    io->write(createWebsocketReply(websockets_parse(sock).left(125), Socket::OpCodePong));
                    return;
                    // pong
                case Socket::OpCodePong:
                    sock->websocketContext->request()->websocketPong(websockets_parse(sock),
                                                                     0, // TODO
                                                                     sock->websocketContext);
                    return;
                default:
                    break;
                }
                // reset the status
                sock->websocket_phase = 0;
                sock->websocket_need = 2;
                // decapitate the buffer
            {
                sock->buf_size -= sock->websocket_pktsize;
                memmove(sock->buffer, sock->buffer + sock->websocket_pktsize, sock->buf_size);
            }
                //                if (uwsgi_buffer_decapitate(sock->websocket_buf, sock->websocket_pktsize)) return NULL;
                break;
                // oops
            default:
                //                uwsgi_log("[uwsgi-websocket] \"%.*s %.*s\" (%.*s) BUG error in websocket parser\n", REQ_DATA);
                return ;
            }
        }
        // need more data
        else {
            qDebug() << "need more data";
            return;
            //            if (uwsgi_buffer_ensure(sock->websocket_buf, uwsgi.page_size)) return NULL;
            //            ssize_t len = uwsgi_websockets_recv_pkt(sock, nb);
            //            if (len <= 0) {
            //                if (nb == 1 && len == 0) {
            //                    // return an empty buffer to signal blocking event
            //                    return uwsgi_buffer_new(0);
            //                }
            //                return NULL;
            //            }
            //            // update buffer size
            //            sock->websocket_buf->pos+=len;
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
