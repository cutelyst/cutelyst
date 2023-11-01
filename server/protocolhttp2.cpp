/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "protocolhttp2.h"

#include "hpack.h"
#include "server.h"
#include "socket.h"

#include <QEventLoop>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(CWSGI_H2, "cwsgi.http2", QtWarningMsg)

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
};

enum SettingsFlags { FlagSettingsAck = 0x1 };

enum PingFlags { FlagPingAck = 0x1 };

enum HeaderFlags {
    FlagHeadersEndStream  = 0x1,
    FlagHeadersEndHeaders = 0x4,
    FlagHeadersPadded     = 0x8,
    FlagHeadersPriority   = 0x20,
};

enum PushPromiseFlags {
    FlagPushPromiseEndHeaders = 0x4,
    FlagPushPromisePadded     = 0x8,
};

enum DataFlags {
    FlagDataEndStream = 0x1,
    FlagDataPadded    = 0x8,
};

enum FrameType {
    FrameData         = 0x0,
    FrameHeaders      = 0x1,
    FramePriority     = 0x2,
    FrameRstStream    = 0x3,
    FrameSettings     = 0x4,
    FramePushPromise  = 0x5,
    FramePing         = 0x6,
    FrameGoaway       = 0x7,
    FrameWindowUpdate = 0x8,
    FrameContinuation = 0x9
};

enum ErrorCodes {
    ErrorNoError            = 0x0,
    ErrorProtocolError      = 0x1,
    ErrorInternalError      = 0x2,
    ErrorFlowControlError   = 0x3,
    ErrorSettingsTimeout    = 0x4,
    ErrorStreamClosed       = 0x5,
    ErrorFrameSizeError     = 0x6,
    ErrorRefusedStream      = 0x7,
    ErrorCancel             = 0x8,
    ErrorCompressionError   = 0x9,
    ErrorConnectError       = 0xA,
    ErrorEnhanceYourCalm    = 0xB,
    ErrorInadequateSecurity = 0xC,
    ErrorHttp11Required     = 0xD
};

enum Settings {
    SETTINGS_HEADER_TABLE_SIZE       = 0x1,
    SETTINGS_ENABLE_PUSH             = 0x2,
    SETTINGS_MAX_CONCURRENT_STREAMS  = 0x3,
    SETTINGS_INITIAL_WINDOW_SIZE     = 0x4,
    SETTINGS_MAX_FRAME_SIZE          = 0x5,
    SETTINGS_MAX_HEADER_LIST_SIZE    = 0x6,
    SETTINGS_ENABLE_CONNECT_PROTOCOL = 0x8,
};

#define PREFACE_SIZE 24

ProtocolHttp2::ProtocolHttp2(Server *wsgi)
    : Protocol(wsgi)
    , m_headerTableSize(qint32(wsgi->http2HeaderTableSize()))
{
    m_bufferSize = qMin(m_bufferSize, 2147483647);

    // 2^14 + 9 (octects)
    if (m_bufferSize < 16393) {
        qFatal("HTTP/2 Protocol requires that buffer-size to be at least '16393' in size, current "
               "value is '%s'",
               QByteArray::number(m_bufferSize).constData());
    }

    m_maxFrameSize = quint32(m_bufferSize - 9);
}

ProtocolHttp2::~ProtocolHttp2()
{
}

Protocol::Type ProtocolHttp2::type() const
{
    return Protocol::Type::Http2;
}

void ProtocolHttp2::parse(Socket *sock, QIODevice *io) const
{
    auto request = static_cast<ProtoRequestHttp2 *>(sock->protoData);

    qint64 bytesAvailable = io->bytesAvailable();
    //    qCDebug(CWSGI_H2) << sock << "READ available" << bytesAvailable << "buffer size" <<
    //    request->buf_size << "default buffer size" << m_bufferSize ;

    do {
        const qint64 len =
            io->read(request->buffer + request->buf_size, m_bufferSize - request->buf_size);
        bytesAvailable -= len;

        if (len > 0) {
            request->buf_size += len;
            int ret = 0;
            while (request->buf_size && ret == 0) {
                //                qDebug() << "Current buffer size" << request->connState <<
                //                request->buf_size;//QByteArray(request->buffer,
                //                request->buf_size);
                if (request->connState == ProtoRequestHttp2::MethodLine) {
                    if (request->buf_size >= PREFACE_SIZE) {
                        if (memcmp(request->buffer, "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n", 24) == 0) {
                            //                            qCDebug(CWSGI_H2) << "Got MAGIC" <<
                            //                            sizeof(struct h2_frame);
                            request->buf_size -= PREFACE_SIZE;
                            memmove(request->buffer,
                                    request->buffer + PREFACE_SIZE,
                                    size_t(request->buf_size));
                            request->connState = ProtoRequestHttp2::H2Frames;

                            sendSettings(io,
                                         {
                                             {SETTINGS_ENABLE_CONNECT_PROTOCOL, 0},
                                             {SETTINGS_MAX_FRAME_SIZE, m_maxFrameSize},
                                             {SETTINGS_HEADER_TABLE_SIZE, m_headerTableSize},
                                         });
                        } else {
                            qCDebug(CWSGI_H2) << "Protocol Error: Invalid connection preface"
                                              << sock->remoteAddress.toString();
                            // RFC 7540 says this MAY be omitted, so let's reduce further processing
                            //                            ret = sendGoAway(io, request->maxStreamId,
                            //                            ErrorProtocolError);
                            sock->connectionClose();
                            return;
                        }
                    } else {
                        //                        qDebug() << "MAGIC needs more data" <<
                        //                        bytesAvailable;
                        break;
                    }
                } else if (request->connState == ProtoRequestHttp2::H2Frames) {
                    if (request->buf_size >= int(sizeof(struct h2_frame))) {
                        auto fr = reinterpret_cast<struct h2_frame *>(request->buffer);
                        H2Frame frame;
                        frame.len = quint32(fr->size0 | (fr->size1 << 8) | (fr->size2 << 16));
                        frame.streamId =
                            quint32(fr->rbit_stream_id0 | (fr->rbit_stream_id1 << 8) |
                                    (fr->rbit_stream_id2 << 16) |
                                    ((fr->rbit_stream_id3 & ~0x80) << 24)); // Ignore first bit
                        frame.type  = fr->type;
                        frame.flags = fr->flags;
                        request->pktsize =
                            quint32(fr->size0 | (fr->size1 << 8) | (fr->size2 << 16));
                        request->stream_id =
                            quint32(fr->rbit_stream_id0 | (fr->rbit_stream_id1 << 8) |
                                    (fr->rbit_stream_id2 << 16) |
                                    ((fr->rbit_stream_id3 & ~0x80) << 24)); // Ignore first bit

                        //                        qDebug() << "Frame type" << fr->type
                        //                                 << "flags" << fr->flags
                        //                                 << "stream-id" << request->stream_id
                        //                                 << "required size" << request->pktsize
                        //                                 << "available" << (request->buf_size -
                        //                                 sizeof(struct h2_frame));

                        if (frame.streamId && !(frame.streamId & 1)) {
                            ret = sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                            break;
                        }

                        if (request->pktsize > m_maxFrameSize) {
                            //                            qDebug() << "Frame too big" <<
                            //                            request->pktsize << m_bufferSize;
                            ret = sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
                            break;
                        }

                        if (request->pktsize >
                            (quint32(request->buf_size) - sizeof(struct h2_frame))) {
                            //                            qDebug() << "need more data" <<
                            //                            bytesAvailable;
                            break;
                        }

                        if (request->streamForContinuation) {
                            if (fr->type == FrameContinuation &&
                                request->streamForContinuation == frame.streamId) {
                                fr->type = FrameHeaders;
                            } else {
                                ret = sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                                break;
                            }
                        }

                        if (fr->type == FrameSettings) {
                            ret = parseSettings(request, io, frame);
                        } else if (fr->type == FramePriority) {
                            ret = parsePriority(request, io, frame);
                        } else if (fr->type == FrameHeaders) {
                            ret = parseHeaders(request, io, frame);
                        } else if (fr->type == FramePing) {
                            ret = parsePing(request, io, frame);
                        } else if (fr->type == FrameData) {
                            ret = parseData(request, io, frame);
                        } else if (fr->type == FramePushPromise) {
                            // Client can not PUSH
                            ret = sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                            break;
                        } else if (fr->type == FrameRstStream) {
                            ret = parseRstStream(request, io, frame);
                        } else if (fr->type == FrameWindowUpdate) {
                            ret = parseWindowUpdate(request, io, frame);
                        } else if (fr->type == FrameGoaway) {
                            sock->connectionClose();
                            return;
                        } else if (fr->type == FrameContinuation) {
                            ret = sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                            break;
                        } else {
                            qCDebug(CWSGI_H2) << "Unknown frame type" << fr->type;
                            // Implementations MUST ignore and discard any frame that has a type
                            // that is unknown.
                        }

                        request->buf_size -= 9 + request->pktsize;
                        memmove(request->buffer,
                                request->buffer + 9 + request->pktsize,
                                size_t(request->buf_size));
                    }
                }
            }

            if (ret) {
                //                qDebug() << "Got error closing" << ret;
                sock->connectionClose();
            }
        } else {
            qCWarning(CWSGI_H2) << "Failed to read from socket" << io->errorString();
            break;
        }
    } while (bytesAvailable);
}

ProtocolData *ProtocolHttp2::createData(Socket *sock) const
{
    return new ProtoRequestHttp2(sock, m_bufferSize);
}

int ProtocolHttp2::parseSettings(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const
{
    //    qDebug() << "Consumming SETTINGS";
    if ((fr.flags & FlagSettingsAck && fr.len) || fr.len % 6) {
        sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
        return 1;
    } else if (fr.streamId) {
        sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        return 1;
    }

    if (!(fr.flags & FlagSettingsAck)) {
        QVector<std::pair<quint16, quint32>> settings;
        uint pos = 0;
        while (request->pktsize > pos) {
            quint16 identifier = net_be16(request->buffer + 9 + pos);
            quint32 value      = net_be32(request->buffer + 9 + 2 + pos);
            settings.push_back({identifier, value});
            pos += 6;
            //            qDebug() << "SETTINGS" << identifier << value;
            if (identifier == SETTINGS_ENABLE_PUSH) {
                if (value > 1) {
                    return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                }

                request->canPush = value;
            } else if (identifier == SETTINGS_INITIAL_WINDOW_SIZE) {
                if (value > 2147483647) {
                    return sendGoAway(io, request->maxStreamId, ErrorFlowControlError);
                }

                const qint32 difference = qint32(value) - request->settingsInitialWindowSize;
                request->settingsInitialWindowSize = qint32(value);

                auto it = request->streams.begin();
                while (it != request->streams.end()) {
                    (*it)->windowSize += difference;
                    (*it)->windowUpdated();
                    //                    qCDebug(CWSGI_H2) << "updating stream" << it.key() << "to
                    //                    window" << (*it)->windowSize;
                    ++it;
                }
            } else if (identifier == SETTINGS_MAX_FRAME_SIZE) {
                if (value < 16384 || value > 16777215) {
                    return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
                }
                request->settingsMaxFrameSize = value;
            }
        }
        sendSettingsAck(io);
    }

    return ErrorNoError;
}

int ProtocolHttp2::parseData(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const
{
    //    qCDebug(CWSGI_H2) << "Consuming DATA" << fr.len;
    if (fr.streamId == 0) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    }

    quint8 padLength = 0;
    if (fr.flags & FlagDataPadded) {
        padLength = quint8(*(request->buffer + 9));
        if (padLength >= fr.len) {
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }
    }

    H2Stream *stream;
    auto streamIt = request->streams.constFind(fr.streamId);
    if (streamIt != request->streams.constEnd()) {
        stream = streamIt.value();

        if (stream->state == H2Stream::Idle) {
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        } else if (stream->state == H2Stream::HalfClosed || stream->state == H2Stream::Closed) {
            return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
        }
    } else {
        return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
    }

    //    qCDebug(CWSGI_H2) << "Frame data" << padLength << "state" << stream->state <<
    //    "content-length" << stream->contentLength;

    if (!stream->body) {
        stream->body = createBody(request->contentLength);
        if (!stream->body) {
            // Failed to create body to store data
            return sendGoAway(io, request->maxStreamId, ErrorInternalError);
        }
    }
    stream->body->write(request->buffer + 9, fr.len - padLength);

    stream->consumedData += fr.len - padLength;
    if (stream->contentLength != -1 &&
        ((fr.flags & FlagDataEndStream && stream->contentLength != stream->consumedData) ||
         (stream->contentLength > stream->consumedData))) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    }

    if (fr.flags & FlagDataEndStream) {
        queueStream(request->sock, stream);
    }

    return ErrorNoError;
}

int ProtocolHttp2::parseHeaders(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const
{
    //    qCDebug(CWSGI_H2) << "Consumming HEADERS" << bool(fr.flags & FlagHeadersEndStream);
    if (fr.streamId == 0) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    }
    if (fr.len > request->settingsMaxFrameSize) {
        return sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
    }
    int pos          = 0;
    char *ptr        = request->buffer + 9;
    quint8 padLength = 0;
    if (fr.flags & FlagHeadersPadded) {
        padLength = quint8(*(ptr + pos));
        if (padLength > fr.len) {
            //            qCDebug(CWSGI_H2) << "header pad length";
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }

        pos += 1;
    }

    quint32 streamDependency = 0;
    //    quint8 weight = 0;
    if (fr.flags & FlagHeadersPriority) {
        // TODO disable exclusive bit
        streamDependency = net_be32(ptr + pos);
        if (fr.streamId == streamDependency) {
            //            qCDebug(CWSGI_H2) << "header stream dep";
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }

        pos += 4;
        //        weight = quint8(*(ptr + pos)) + 1;
        pos += 1;
    }
    ptr += pos;

    H2Stream *stream;
    auto streamIt = request->streams.constFind(fr.streamId);
    if (streamIt != request->streams.constEnd()) {
        stream = streamIt.value();

        //        qCDebug(CWSGI_H2) << "------------" << !(fr.flags & FlagHeadersEndStream) <<
        //        stream->state << request->streamForContinuation ;

        if (!(fr.flags & FlagHeadersEndStream) && stream->state == H2Stream::Open &&
            request->streamForContinuation == 0) {
            qCDebug(CWSGI_H2) << "header FlagHeadersEndStream stream->headers.size()";
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }
        if (stream->state == H2Stream::HalfClosed && request->streamForContinuation == 0) {
            return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
        }
        if (stream->state == H2Stream::Closed) {
            return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
        }
    } else {
        if (request->maxStreamId >= fr.streamId) {
            //            qCDebug(CWSGI_H2) << "header maxStreamId ";
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }
        request->maxStreamId = fr.streamId;

        stream = new H2Stream(fr.streamId, request->settingsInitialWindowSize, request);
        if (useStats) {
            stream->startOfRequest = std::chrono::steady_clock::now();
        }
        request->streams.insert(fr.streamId, stream);
    }

    if (stream->state == H2Stream::Idle) {
        stream->state = H2Stream::Open;
    }

    if (fr.flags & FlagHeadersEndStream) {
        stream->state = H2Stream::HalfClosed;
    }

    if (!request->hpack) {
        request->hpack = new HPack(m_headerTableSize);
    }

    if (fr.flags & FlagHeadersEndHeaders) {
        request->streamForContinuation = 0;
        if (!request->headersBuffer.isEmpty()) {
            request->headersBuffer.append(ptr, qint32(fr.len) - pos - padLength);
        }
    } else {
        //        qCDebug(CWSGI_H2) << "Setting HEADERS for continuation on stream" << fr.streamId;
        request->streamForContinuation = fr.streamId;
        request->headersBuffer.append(ptr, qint32(fr.len) - pos - padLength);
        return 0;
    }

    quint8 *it;
    quint8 *itEnd;
    if (request->headersBuffer.size()) {
        it    = reinterpret_cast<quint8 *>(request->headersBuffer.begin());
        itEnd = reinterpret_cast<quint8 *>(request->headersBuffer.end());
    } else {
        it    = reinterpret_cast<quint8 *>(ptr);
        itEnd = it + fr.len - pos - padLength;
    }

    int ret = request->hpack->decode(it, itEnd, stream);
    if (ret) {
        //        qDebug() << "Headers parser error" << ret << QByteArray(ptr + pos, fr.len - pos -
        //        padLength).toHex();
        return sendGoAway(io, request->maxStreamId, quint32(ret));
    }

    //    qDebug() << "Headers" << padLength << streamDependency << weight << "stream headers size"
    //    << stream->headers /*<< QByteArray(ptr + pos, fr.len - pos - padLength).toHex()*/ << ret;

    if ((stream->state == H2Stream::HalfClosed || fr.flags & FlagHeadersEndStream) &&
        request->streamForContinuation == 0) {

        // Process request
        queueStream(request->sock, stream);
    }

    return 0;
}

int ProtocolHttp2::parsePriority(ProtoRequestHttp2 *sock, QIODevice *io, const H2Frame &fr) const
{
    //    qDebug() << "Consumming PRIORITY";
    if (fr.len != 5) {
        return sendGoAway(io, sock->maxStreamId, ErrorFrameSizeError);
    } else if (fr.streamId == 0) {
        return sendGoAway(io, sock->maxStreamId, ErrorProtocolError);
    }

    uint pos = 0;
    while (fr.len > pos) {
        // TODO store/disable EXCLUSIVE bit
        quint32 exclusiveAndStreamDep = net_be32(sock->buffer + 9 + pos);
        //        quint8 weigth = *reinterpret_cast<quint8 *>(sock->buffer + 9 + 4 + pos) + 1;
        //                            settings.push_back({ identifier, value });
        //                            sock->pktsize -= 6;

        if (fr.streamId == exclusiveAndStreamDep) {
            //            qDebug() << "PRIO error2" << exclusiveAndStreamDep << fr.streamId;

            return sendGoAway(io, sock->maxStreamId, ErrorProtocolError);
        }

        pos += 6;
        //        qDebug() << "PRIO" << exclusiveAndStreamDep << weigth;
    }

    return 0;
}

int ProtocolHttp2::parsePing(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const
{
    //    qCDebug(CWSGI_H2) << "Got PING" << fr.flags;
    if (fr.len != 8) {
        return sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
    } else if (fr.streamId) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    }

    if (!(fr.flags & FlagPingAck)) {
        sendPing(io, FlagPingAck, request->buffer + 9, 8);
    }
    return 0;
}

int ProtocolHttp2::parseRstStream(ProtoRequestHttp2 *request,
                                  QIODevice *io,
                                  const H2Frame &fr) const
{
    //    qCDebug(CWSGI_H2) << "Consuming RST_STREAM";

    if (fr.streamId == 0) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    } else if (request->pktsize != 4) {
        return sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
    }

    H2Stream *stream;
    auto streamIt = request->streams.constFind(fr.streamId);
    if (streamIt != request->streams.constEnd()) {
        stream = streamIt.value();

        //        qCDebug(CWSGI_H2) << "Consuming RST_STREAM state" << stream->state;
        if (stream->state == H2Stream::Idle) {
            return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
        }

    } else {
        return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
    }

    stream->state = H2Stream::Closed;

    //    quint32 errorCode = h2_be32(request->buffer + 9);
    //    qCDebug(CWSGI_H2) << "RST frame" << errorCode;

    return 0;
}

int ProtocolHttp2::parseWindowUpdate(ProtoRequestHttp2 *request,
                                     QIODevice *io,
                                     const H2Frame &fr) const
{
    if (fr.len != 4) {
        return sendGoAway(io, request->maxStreamId, ErrorFrameSizeError);
    }

    quint32 windowSizeIncrement = net_be32(request->buffer + 9);
    if (windowSizeIncrement == 0) {
        return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
    }

    //    qDebug() << "Consuming WINDOW_UPDATE" << fr.streamId << "increment" << windowSizeIncrement
    //    << request;

    if (fr.streamId) {
        H2Stream *stream;
        auto streamIt = request->streams.constFind(fr.streamId);
        if (streamIt != request->streams.constEnd()) {
            stream = streamIt.value();

            if (stream->state == H2Stream::Idle) {
                return sendGoAway(io, request->maxStreamId, ErrorProtocolError);
            }
        } else {
            return sendGoAway(io, request->maxStreamId, ErrorStreamClosed);
        }

        const qint64 result = qint64(stream->windowSize) + windowSizeIncrement;
        if (result > 2147483647) {
            stream->state = H2Stream::Closed;
            return sendRstStream(io, fr.streamId, ErrorFlowControlError);
        }
        stream->windowSize = qint32(result);
        stream->windowUpdated();

    } else {
        const qint64 result = qint64(request->windowSize) + windowSizeIncrement;
        if (result > 2147483647) {
            return sendGoAway(io, request->maxStreamId, ErrorFlowControlError);
        }
        request->windowSize = qint32(result);

        if (result > 0) {
            auto streamIt = request->streams.constBegin();
            if (streamIt != request->streams.constEnd()) {
                (*streamIt)->windowUpdated();
                ++streamIt;
            }
        }
    }

    return 0;
}

int ProtocolHttp2::sendGoAway(QIODevice *io, quint32 lastStreamId, quint32 error) const
{
    //    qDebug() << "GOAWAY" << error;
    QByteArray data;
    data.append(char(lastStreamId >> 24));
    data.append(char(lastStreamId >> 16));
    data.append(char(lastStreamId >> 8));
    data.append(char(lastStreamId));
    data.append(char(error >> 24));
    data.append(char(error >> 16));
    data.append(char(error >> 8));
    data.append(char(error));
    //    quint64 data = error;
    //    sendFrame(io, FrameGoaway, 0, 0, reinterpret_cast<const char *>(&data), 4);
    int ret = sendFrame(io, FrameGoaway, 0, 0, data.constData(), 8);
    //    qDebug() << ret << int(error);
    return error || ret;
}

int ProtocolHttp2::sendRstStream(QIODevice *io, quint32 streamId, quint32 error) const
{
    //    qDebug() << "RST_STREAM" << streamId << error;
    QByteArray data;
    data.append(char(error >> 24));
    data.append(char(error >> 16));
    data.append(char(error >> 8));
    data.append(char(error));
    //    quint64 data = error;
    //    sendFrame(io, FrameGoaway, 0, 0, reinterpret_cast<const char *>(&data), 4);
    int ret = sendFrame(io, FrameRstStream, 0, streamId, data.constData(), 4);
    //    qDebug() << ret << int(error);
    return error || ret;
}

int ProtocolHttp2::sendSettings(QIODevice *io,
                                const std::vector<std::pair<quint16, quint32>> &settings) const
{
    QByteArray data;
    for (const std::pair<quint16, quint32> &pair : settings) {
        data.append(char(pair.first >> 8));
        data.append(char(pair.first));
        data.append(char(pair.second >> 24));
        data.append(char(pair.second >> 16));
        data.append(char(pair.second >> 8));
        data.append(char(pair.second));
    }
    //    qDebug() << "Send settings" << data.toHex();
    return sendFrame(io, FrameSettings, 0, 0, data.constData(), data.length());
}

int ProtocolHttp2::sendSettingsAck(QIODevice *io) const
{
    return sendFrame(io, FrameSettings, FlagSettingsAck);
}

int ProtocolHttp2::sendPing(QIODevice *io, quint8 flags, const char *data, qint32 dataLen) const
{
    return sendFrame(io, FramePing, flags, 0, data, dataLen);
}

int ProtocolHttp2::sendData(QIODevice *io,
                            quint32 streamId,
                            qint32 windowSize,
                            const char *data,
                            qint32 dataLen) const
{
    if (windowSize < 1) {
        //        qDebug() << "Window size too small, holding";
        return 0;
    }
    if (windowSize < dataLen) {
        qint32 i     = 0;
        quint8 flags = 0;
        while (i < dataLen) {
            int ret = sendFrame(io, FrameData, flags, streamId, data + i, windowSize);
            if (ret) {
                return ret;
            }
            i += windowSize;
            if ((i + 1) == dataLen) {
                flags = FlagDataEndStream;
            }
        }
        return 0;
    } else {
        return sendFrame(io, FrameData, FlagDataEndStream, streamId, data, dataLen);
    }
}

int ProtocolHttp2::sendFrame(QIODevice *io,
                             quint8 type,
                             quint8 flags,
                             quint32 streamId,
                             const char *data,
                             qint32 dataLen) const
{
    h2_frame fr;

    fr.size2           = quint8(dataLen >> 16);
    fr.size1           = quint8(dataLen >> 8);
    fr.size0           = quint8(dataLen);
    fr.type            = type;
    fr.flags           = flags;
    fr.rbit_stream_id3 = quint8(streamId >> 24);
    fr.rbit_stream_id2 = quint8(streamId >> 16);
    fr.rbit_stream_id1 = quint8(streamId >> 8);
    fr.rbit_stream_id0 = quint8(streamId);

    //    qCDebug(CWSGI_H2) << "Sending frame"
    //                      << type
    //                      << flags
    //                      << streamId
    //                      << dataLen;

    //    qCDebug(CWSGI_H2) << "Frame" << QByteArray(reinterpret_cast<const char *>(&fr),
    //    sizeof(struct h2_frame)).toHex();
    if (io->write(reinterpret_cast<const char *>(&fr), sizeof(struct h2_frame)) !=
        sizeof(struct h2_frame)) {
        return -1;
    }
    //    qCDebug(CWSGI_H2) << "Frame data" << QByteArray(data, dataLen).toHex();
    if (dataLen && io->write(data, dataLen) != dataLen) {
        return -1;
    }
    return 0;
}

void ProtocolHttp2::queueStream(Socket *socket, H2Stream *stream) const
{
    ++socket->processing;
    if (stream->body) {
        stream->body->seek(0);
    }
    Q_EMIT socket->engine->processRequestAsync(stream);
}

bool ProtocolHttp2::upgradeH2C(Socket *socket,
                               QIODevice *io,
                               const Cutelyst::EngineRequest &request)
{
    const Cutelyst::Headers &headers = request.headers;
    if (headers.header("Upgrade").compare("h2c") == 0 &&
        headers.connection().compare("Upgrade, HTTP2-Settings") == 0) {
        const auto settings = headers.header("Http2-Settings");
        if (!settings.isEmpty()) {
            io->write("HTTP/1.1 101 Switching Protocols\r\n"
                      "Connection: Upgrade\r\n"
                      "Upgrade: h2c\r\n\r\n");
            socket->proto              = this;
            auto protoRequest          = new ProtoRequestHttp2(socket, m_bufferSize);
            protoRequest->upgradedFrom = socket->protoData;
            socket->protoData          = protoRequest;

            protoRequest->hpack       = new HPack(m_headerTableSize);
            protoRequest->maxStreamId = 1;

            auto stream            = new H2Stream(1, 65535, protoRequest);
            stream->method         = request.method;
            stream->path           = request.path;
            stream->query          = request.query;
            stream->remoteUser     = request.remoteUser;
            stream->headers        = request.headers;
            stream->startOfRequest = std::chrono::steady_clock::now();
            stream->status         = request.status;
            stream->body           = request.body;

            stream->state = H2Stream::HalfClosed;
            protoRequest->streams.insert(1, stream);
            protoRequest->maxStreamId = 1;

            sendSettings(io,
                         {
                             {SETTINGS_MAX_FRAME_SIZE, m_maxFrameSize},
                             {SETTINGS_HEADER_TABLE_SIZE, m_headerTableSize},
                         });

            // Process request
            queueStream(socket, stream);
            qCDebug(CWSGI_H2) << "upgraded";
            return true;
        }
    }
    return false;
}

ProtoRequestHttp2::ProtoRequestHttp2(Cutelyst::Socket *sock, int bufferSize)
    : ProtocolData(sock, bufferSize)
{
}

ProtoRequestHttp2::~ProtoRequestHttp2()
{
}

void ProtoRequestHttp2::setupNewConnection(Socket *sock)
{
    Q_UNUSED(sock)
}

H2Stream::H2Stream(quint32 _streamId, qint32 _initialWindowSize, ProtoRequestHttp2 *protoRequestH2)
    : protoRequest(protoRequestH2)
    , streamId(_streamId)
    , windowSize(_initialWindowSize)
{
    protocol      = "HTTP/2"_qba;
    serverAddress = protoRequestH2->sock->serverAddress;
    remoteAddress = protoRequestH2->sock->remoteAddress;
    remotePort    = protoRequestH2->sock->remotePort;
    isSecure      = protoRequestH2->sock->isSecure;
}

H2Stream::~H2Stream()
{
    if (loop) {
        loop->exit(-1);
        delete loop;
    }
}

qint64 H2Stream::doWrite(const char *data, qint64 len)
{
    int ret     = -1;
    auto parser = dynamic_cast<ProtocolHttp2 *>(protoRequest->sock->proto);

    qint64 remainingData = len;
    qint64 sent          = 0;
    while (remainingData > 0 && state != H2Stream::Closed) {
        qint64 availableWindowSize = qMin(windowSize, protoRequest->windowSize);
        //        qCDebug(CWSGI_H2) << "H2Stream::doWrite" << len << streamId <<
        //        "availableWindowSize" << availableWindowSize
        //                          << "remaining data" << remainingData
        //                          << "stream" << this << protoRequest;
        availableWindowSize =
            qMin(availableWindowSize, (qint64) protoRequest->settingsMaxFrameSize);
        if (availableWindowSize == 0) {
            if (!loop) {
                loop = new QEventLoop;
            }
            if (loop->exec() == 0) {
                continue;
            }
            return -1;
        }

        //        qCDebug(CWSGI_H2) << "H2Stream::doWrite" << len << streamId <<
        //        "availableWindowSize" << availableWindowSize
        //                          << "remaining data" << remainingData;

        if (availableWindowSize > remainingData) {
            ret           = parser->sendFrame(protoRequest->io,
                                    FrameData,
                                    FlagDataEndStream,
                                    streamId,
                                    data + sent,
                                    qint32(remainingData));
            remainingData = 0;
            protoRequest->windowSize -= remainingData;
            windowSize -= remainingData;
            sent += remainingData;
        } else {
            ret = parser->sendFrame(protoRequest->io,
                                    FrameData,
                                    0x0,
                                    streamId,
                                    data + sent,
                                    qint32(availableWindowSize));
            remainingData -= availableWindowSize;
            protoRequest->windowSize -= availableWindowSize;
            windowSize -= availableWindowSize;
            sent += availableWindowSize;
        }

        //        qCDebug(CWSGI_H2) << "H2Stream::doWrite ret" << ret << (ret == 0 ? len : -1);
    }

    return ret == 0 ? len : -1;
}

bool H2Stream::writeHeaders(quint16 status, const Cutelyst::Headers &headers)
{
    QByteArray buf;
    protoRequest->hpack->encodeHeaders(
        status, headers, buf, static_cast<ServerEngine *>(protoRequest->sock->engine));

    auto parser = dynamic_cast<ProtocolHttp2 *>(protoRequest->sock->proto);

    int ret = parser->sendFrame(protoRequest->io,
                                FrameHeaders,
                                FlagHeadersEndHeaders,
                                streamId,
                                buf.constData(),
                                buf.size());

    return ret == 0;
}

void H2Stream::processingFinished()
{
    state = Closed;
    protoRequest->streams.remove(streamId);
    protoRequest->sock->requestFinished();
    delete this;
}

void H2Stream::windowUpdated()
{
    //    qDebug() << "WINDOW_UPDATED" << protoRequest->windowSize << windowSize << loop << (loop &&
    //    loop->isRunning()) << this << protoRequest;

    if (protoRequest->windowSize > 0 && windowSize > 0 && loop && loop->isRunning()) {
        loop->quit();
    }
}

#include "moc_protocolhttp2.cpp"
