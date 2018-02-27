/*
 * Copyright (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "protocolhttp2.h"

#include "socket.h"

#include <QLoggingCategory>

using namespace CWSGI;

Q_LOGGING_CATEGORY(CWSGI_H2, "cwsgi.http2")

#ifdef Q_CC_MSVC
#pragma pack(push)
#pragma pack(1)
#endif
struct h2_frame {
    quint8 size2;
    quint8 size1;
    quint8 size0;
    quint8 type;
    quint8 flags;
    quint8 rbit_stream_id3;
    quint8 rbit_stream_id2;
    quint8 rbit_stream_id1;
    quint8 rbit_stream_id0;
}
#ifdef Q_CC_MSVC
;
#pragma pack(pop)
#else
__attribute__ ((__packed__));
#endif

enum Flags {
    FlagSettingsAck = 0x1,
    FlagPingAck = 0x1
};

enum FrameType {
    FrameData = 0x0,
    FrameHeaders = 0x1,
    FramePriority = 0x2,
    FrameRstStream = 0x3,
    FrameSettings = 0x4,
    FramePushPromise = 0x5,
    FramePing = 0x6,
    FrameGoaway = 0x7,
    FrameWindowUpdate = 0x8,
    FrameContinuation = 0x9
};

enum ErrorCodes {
    ErrorNoError = 0x0,
    ErrorProtocolError = 0x1,
    ErrorInternalError = 0x2,
    ErrorFlowControlError = 0x3,
    ErrorSettingsTimeout = 0x4,
    ErrorStreamClosed = 0x5,
    ErrorFrameSizeError = 0x6,
    ErrorRefusedStream = 0x7,
    ErrorCancel = 0x8,
    ErrorCompressionError = 0x9,
    ErrorConnectError = 0xA,
    ErrorEnhanceYourCalm  = 0xB,
    ErrorInadequateSecurity = 0xC,
    ErrorHttp11Required = 0xD
};

enum Settings {
    SETTINGS_HEADER_TABLE_SIZE = 0x1,
    SETTINGS_ENABLE_PUSH = 0x2,
    SETTINGS_MAX_CONCURRENT_STREAMS = 0x3,
    SETTINGS_INITIAL_WINDOW_SIZE = 0x4,
    SETTINGS_MAX_FRAME_SIZE = 0x5,
    SETTINGS_MAX_HEADER_LIST_SIZE = 0x6
};

quint32 h2_be32(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint32 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[2] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[3] = static_cast<quint8>(*src & 0xff);
    return ret;
}

quint32 h2_be24(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint32 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[2] = static_cast<quint8>(*src & 0xff);
    return ret;
}

quint16 h2_be16(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint16 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[1] = static_cast<quint8>(*src & 0xff);
    return ret;
}

#define PREFACE_SIZE 24

ProtocolHttp2::ProtocolHttp2(WSGI *wsgi) : Protocol(wsgi)
{

}

ProtocolHttp2::~ProtocolHttp2()
{

}

Protocol::Type ProtocolHttp2::type() const
{
    return Http2;
}

void ProtocolHttp2::readyRead(Socket *sock, QIODevice *io) const
{
    qint64 bytesAvailable = io->bytesAvailable();
    qCDebug(CWSGI_H2) << "readyRead available" << bytesAvailable;

    int len = io->read(sock->buffer + sock->buf_size, m_bufferSize - sock->buf_size);
    bytesAvailable -= len;

    if (len > 0) {
        sock->buf_size += len;

        while (sock->buf_size) {
            qDebug() << QByteArray(sock->buffer, sock->buf_size);
            if (sock->connState == Socket::MethodLine) {
                if (sock->buf_size >= PREFACE_SIZE) {
                    if (memcmp(sock->buffer, "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n", 24) == 0) {
                        qCDebug(CWSGI_H2) << "Got preface" << sizeof(struct h2_frame);
                        memmove(sock->buffer, sock->buffer + PREFACE_SIZE, sock->buf_size - PREFACE_SIZE);
                        sock->buf_size -= PREFACE_SIZE;
                        sock->connState = Socket::H2Frames;

                        std::vector<std::pair<quint16, quint32>> settings;
                        sendSettings(io, settings);
                    } else {
                        qCDebug(CWSGI_H2) << "Wrong preface";
                        sendError(io);
                        sock->connectionClose();
                        return;
                        //                    return PROTOCOL_ERROR;
                    }
                }
            } else if (sock->connState == Socket::H2Frames) {
                if (sock->buf_size >= sizeof(struct h2_frame)) {
                    auto fr = reinterpret_cast<struct h2_frame *>(sock->buffer);
                    sock->pktsize = fr->size0 | (fr->size1 << 8) | (fr->size2 << 16);
                    sock->stream_id = fr->rbit_stream_id0 |
                            (fr->rbit_stream_id1 << 8) |
                            (fr->rbit_stream_id2 << 16) |
                            ((fr->rbit_stream_id3 & ~0x80) << 24); // Ignore first bit

                    qDebug() << "frame type" << fr->type << "flags" << fr->flags << "stream-id" << sock->stream_id;
                    qDebug() << "Reserved bit" << (fr->rbit_stream_id3) << (fr->rbit_stream_id3 & ~0x80) << (fr->rbit_stream_id3 & 0x80);
//                    qDebug() << "frame size" << quint32(fr->size1 | (fr->size2 << 8) | (fr->size3 << 16));
                    qDebug() << "frame size" << sock->pktsize << "available" << (sock->buf_size - sizeof(struct h2_frame));
                    if (sock->pktsize > (sock->buf_size - sizeof(struct h2_frame))) {
                        qDebug() << "need more data" << bytesAvailable;
                        return;
                    }

                    if (fr->type == FrameSettings) {
                        qDebug() << "Consumming settings";
                        QVector<std::pair<quint16, quint32>> settings;
                        uint pos = 0;
                        while (sock->pktsize > pos) {
                            quint16 identifier = h2_be16(sock->buffer + 9 + pos);
                            quint32 value = h2_be32(sock->buffer + 9 + 2 + pos);
                            settings.push_back({ identifier, value });
//                            sock->pktsize -= 6;
                            pos += 6;
                            qDebug() << "SETTINGS" << identifier << value;
                        }
                        sendSettingsAck(io);

                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FramePriority) {
                        qDebug() << "Consumming priority";
                        if (sock->pktsize != 5) {
                            // FRAME_SIZE_ERROR
                            sendError(io);
                            sock->connectionClose();
                            return;
                        }

                        uint pos = 0;
                        while (sock->pktsize > pos) {
                            // TODO store/disable EXCLUSIVE bit
                            quint32 exclusiveAndStreamDep = h2_be32(sock->buffer + 9 + pos);
                            quint8 weigth = *reinterpret_cast<quint8 *>(sock->buffer + 9 + 4 + pos) + 1;
//                            settings.push_back({ identifier, value });
//                            sock->pktsize -= 6;

                            if (exclusiveAndStreamDep == 0) {
                                // PROTOCOL_ERROR
                                sendError(io);
                                sock->connectionClose();
                                return;
                            }

                            pos += 6;
                            qDebug() << "PRIO" << exclusiveAndStreamDep << weigth;
                        }
                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FrameHeaders) {
                        qDebug() << "Consumming headers";
                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FramePing) {
                        qCDebug(CWSGI_H2) << "Got ping" << fr->flags;
                        if (sock->pktsize != 8) {
                            // FRAME_SIZE_ERROR
                            sendError(io);
                            sock->connectionClose();
                            return;
                        } else if (sock->stream_id) {
                            // PROTOCOL_ERROR
                            sendError(io);
                            sock->connectionClose();
                            return;
                        }

                        if (!(fr->flags & FlagPingAck)) {
                            sendPing(io, FlagPingAck, sock->buffer + 9, 8);
                        }
                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FrameData) {
                        qCDebug(CWSGI_H2) << "Frame data" << fr->type;
                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FramePushPromise) {
                        // PROTOCOL_ERROR
                        sendError(io);
                        sock->connectionClose();
                        return;
                    } else if (fr->type == FrameRstStream) {
                        if (sock->stream_id == 0) {
                            // PROTOCOL_ERROR
                            sendError(io);
                            sock->connectionClose();
                            return;
                        } else if (sock->pktsize != 4) {
                            // FRAME_SIZE_ERROR
                            sendError(io);
                            sock->connectionClose();
                            return;
                        }

                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    } else if (fr->type == FrameGoaway) {
                        sock->connectionClose();
                        return;
                    } else {
                        qCDebug(CWSGI_H2) << "Unknown frame type" << fr->type;
                        // Implementations MUST ignore and discard any frame that has a type that is unknown.
                        sock->buf_size -= 9 + sock->pktsize;
                        memmove(sock->buffer, sock->buffer + 9 + sock->pktsize, sock->buf_size);
                    }
                }
            }
        }
    }
}

int ProtocolHttp2::sendError(QIODevice *io) const
{
    return sendFrame(io, FrameGoaway, 0);
}

int ProtocolHttp2::sendSettings(QIODevice *io, const std::vector<std::pair<quint16, quint32>> &settings) const
{
    Q_UNUSED(settings);
    return sendFrame(io, FrameSettings, 0);
}

int ProtocolHttp2::sendSettingsAck(QIODevice *io) const
{
    return sendFrame(io, FrameSettings, FlagSettingsAck);
}

int ProtocolHttp2::sendPing(QIODevice *io, quint8 flags, const char *data, qint32 dataLen) const
{
    return sendFrame(io, FramePing, flags, data, dataLen);
}

int ProtocolHttp2::sendFrame(QIODevice *io, quint8 type, quint8 flags, const char *data, qint32 dataLen) const
{
    h2_frame fr;

    fr.size2 = quint8(dataLen >> 16);
    fr.size1 = quint8(dataLen >> 8);
    fr.size0 = quint8(dataLen);
    fr.type = type;
    fr.flags = flags;
    fr.rbit_stream_id3 = 0;
    fr.rbit_stream_id2 = 0;
    fr.rbit_stream_id1 = 0;
    fr.rbit_stream_id0 = 0;

    qCDebug(CWSGI_H2) << "Sending frame"
                      << type
                      << flags
                      << quint32(fr.size0 | (fr.size1 << 8) | (fr.size2 << 16))
                      << quint32(fr.rbit_stream_id0 | (fr.rbit_stream_id1 << 8) | (fr.rbit_stream_id2 << 16) | (fr.rbit_stream_id3 << 24));

    qCDebug(CWSGI_H2) << "Frame" << QByteArray(reinterpret_cast<const char *>(&fr), sizeof(struct h2_frame));
    if (io->write(reinterpret_cast<const char *>(&fr), sizeof(struct h2_frame)) != sizeof(struct h2_frame)) {
        return -1;
    }
    if (dataLen && io->write(data, dataLen) != dataLen) {
        return -1;
    }
    return 0;
}

bool ProtocolHttp2::sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers)
{
    return false;
}
