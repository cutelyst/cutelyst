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
#include "protocolhttp.h"
#include "socket.h"
#include "protocolwebsocket.h"
#include "wsgi.h"

#include <Cutelyst/Headers>
#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QVariant>
#include <QIODevice>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QBuffer>
#include <QTimer>
#include <QLoggingCategory>

using namespace CWSGI;

Q_LOGGING_CATEGORY(CWSGI_HTTP, "cwsgi.http")
Q_DECLARE_LOGGING_CATEGORY(CWSGI_SOCK)

ProtocolHttp::ProtocolHttp(WSGI *wsgi) : Protocol(wsgi)
  , m_websocketProto(new ProtocolWebSocket(wsgi))
{

}

ProtocolHttp::~ProtocolHttp()
{
}

Protocol::Type ProtocolHttp::type() const
{
    return Http11;
}

inline int CrLfIndexIn(const char *str, int len, int from)
{
    do {
        const char *pch = static_cast<const char *>(memchr(str + from, '\r', len - from));
        if (pch != NULL) {
            int pos = pch - str;
            if ((pos + 1) < len) {
                if (*++pch == '\n') {
                    return pos;
                } else {
                    from = ++pos;
                    continue;
                }
            }
        }
        break;
    } while (true);

    return -1;
}

void ProtocolHttp::readyRead(Socket *sock, QIODevice *io) const
{
    // Post buffering
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    if (protoRequest->connState == ProtoRequestHttp::ContentBody) {
        qint64 bytesAvailable = io->bytesAvailable();
        int len;
        qint64 remaining;

        QIODevice *body /*= protoRequest->body*/;
        do {
            remaining = protoRequest->contentLength - body->size();
            len = io->read(m_postBuffer, qMin(m_postBufferSize, remaining));
            if (len == -1) {
                sock->connectionClose();
                return;
            }
            bytesAvailable -= len;
//            qCDebug(CWSGI_HTTP) << "WRITE body" << protoRequest->contentLength << remaining << len << (remaining == len) << protoRequest->bytesAvailable();
            body->write(m_postBuffer, len);
        } while (bytesAvailable);

        if (remaining == len) {
            processRequest(sock);
        }

        return;
    }

    int len = io->read(protoRequest->buffer + protoRequest->buf_size, m_bufferSize - protoRequest->buf_size);
    if (len == -1) {
        qCWarning(CWSGI_HTTP) << "Failed to read from socket" << io->errorString();
        return;
    }
    protoRequest->buf_size += len;

    while (protoRequest->last < protoRequest->buf_size) {
//        qCDebug(CWSGI_HTTP) << Q_FUNC_INFO << QByteArray(protoRequest->buf, protoRequest->buf_size);
        int ix = CrLfIndexIn(protoRequest->buffer, protoRequest->buf_size, protoRequest->last);
        if (ix != -1) {
            int len = ix - protoRequest->beginLine;
            char *ptr = protoRequest->buffer + protoRequest->beginLine;
            protoRequest->beginLine = ix + 2;
            protoRequest->last = protoRequest->beginLine;

            if (protoRequest->connState == ProtoRequestHttp::MethodLine) {
                if (!protoRequest->startOfRequest) {
                    protoRequest->startOfRequest = sock->engine->time();
                }
                parseMethod(ptr, ptr + len, sock);
                protoRequest->connState = ProtoRequestHttp::HeaderLine;
                protoRequest->contentLength = -1;
                protoRequest->headers = Cutelyst::Headers();
//                qCDebug(CWSGI_HTTP) << "--------" << protoRequest->method << protoRequest->path << protoRequest->query << protoRequest->protocol;

            } else if (protoRequest->connState == ProtoRequestHttp::HeaderLine) {
                if (len) {
                    parseHeader(ptr, ptr + len, sock);
                } else {
                    if (protoRequest->contentLength != -1) {
                        protoRequest->connState = ProtoRequestHttp::ContentBody;
                        if (m_postBuffering && protoRequest->contentLength > m_postBuffering) {
                            auto temp = new QTemporaryFile;
                            if (!temp->open()) {
                                qCWarning(CWSGI_HTTP) << "Failed to open temporary file to store post" << temp->errorString();
                                io->close(); // On error close immediately
                                return;
                            }
                            protoRequest->body = temp;
                        } else if (m_postBuffering && protoRequest->contentLength <= m_postBuffering) {
                            auto buffer = new QBuffer;
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(protoRequest->contentLength);
                            protoRequest->body = buffer;
                        } else {
                            // Unbuffered
                            auto buffer = new QBuffer;
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(protoRequest->contentLength);
                            protoRequest->body = buffer;
                        }

                        ptr += 2;
                        len = qMin(protoRequest->contentLength, static_cast<qint64>(protoRequest->buf_size - protoRequest->last));
//                        qCDebug(CWSGI_HTTP) << "WRITE" << protoRequest->contentLength << len;
                        if (len) {
                            protoRequest->body->write(ptr, len);
                        }
                        protoRequest->last += len;

                        if (protoRequest->contentLength > len) {
//                            qCDebug(CWSGI_HTTP) << "WRITE more..." << protoRequest->contentLength << len;
                            // need to wait for more data
                            return;
                        }
                    }

                    if (!processRequest(sock)) {
                        break;
                    }
                }
            }
        } else {
            if (!protoRequest->startOfRequest) {
                protoRequest->startOfRequest = sock->engine->time();
            }
            protoRequest->last = protoRequest->buf_size;
        }
    }

    if (protoRequest->buf_size == m_bufferSize) {
        // 414 Request-URI Too Long
    }
}

bool ProtocolHttp::sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers)
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    int msgLen;
    const char *msg = CWsgiEngine::httpStatusMessage(status, &msgLen);
    io->write(msg, msgLen);

    const auto headersData = headers.data();
    ProtoRequestHttp::HeaderConnection fallbackConnection = protoRequest->headerConnection;
    protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionNotSet;

    bool hasDate = false;
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
        const QString &key = it.key();
        const QString &value = it.value();
        if (protoRequest->headerConnection == ProtoRequestHttp::HeaderConnectionNotSet && key == QLatin1String("CONNECTION")) {
            if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
                protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionClose;
            } else if (value.compare(QLatin1String("upgrade"), Qt::CaseInsensitive) == 0) {
                protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionUpgrade;
            } else {
                protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionKeep;
            }
        } else if (!hasDate && key == QLatin1String("DATE")) {
            hasDate = true;
        }

        QString ret(QLatin1String("\r\n") + CWsgiEngine::camelCaseHeader(key) + QLatin1String(": ") + value);
        io->write(ret.toLatin1());

        ++it;
    }

    if (protoRequest->headerConnection == ProtoRequestHttp::HeaderConnectionNotSet) {
        if (fallbackConnection == ProtoRequestHttp::HeaderConnectionKeep) {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionKeep;
            io->write("\r\nConnection: keep-alive", 24);
        } else {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionClose;
            io->write("\r\nConnection: close", 19);
        }
    }

    if (!hasDate) {
        io->write(dateHeader);
    }

    return io->write("\r\n\r\n", 4) == 4;
}

bool ProtocolHttp::processRequest(Socket *sock) const
{
    auto request = static_cast<ProtoRequestHttp *>(sock->protoData);
//    qCDebug(CWSGI_HTTP) << "processRequest" << sock->protoData->contentLength;
    sock->processing = true;
    if (request->body) {
        request->body->seek(0);
    }

    Cutelyst::Context *c = sock->engine->processRequest(request);
    sock->processing = false;

    if (request->headerConnection == ProtoRequestHttp::HeaderConnectionUpgrade) {
        // need 2 byte header
        request->websocket_need = 2;
        request->websocket_phase = ProtoRequestHttp::WebSocketPhaseHeaders;
        sock->processing = true;
        request->buf_size = 0;
        sock->proto = m_websocketProto;

        return false; // Must read remaining data
    }
    delete c;

    if (request->headerConnection == ProtoRequestHttp::HeaderConnectionClose) {
        sock->connectionClose();
        return false;
    }

    if (request->last < request->buf_size) {
        // move pipelined request to 0
        int remaining = request->buf_size - request->last;
        memmove(request->buffer, request->buffer + request->last, remaining);
        sock->resetSocket();
        request->buf_size = remaining;

        QCoreApplication::processEvents();
    } else {
        sock->resetSocket();
    }

    return true;
}

void ProtocolHttp::parseMethod(const char *ptr, const char *end, Socket *sock) const
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    const char *word_boundary = ptr;
    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    protoRequest->method = QString::fromLatin1(ptr, word_boundary - ptr);

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    // skip leading slashes
    while (*ptr == '/' && ptr <= end) {
        ++ptr;
    }

    // find path end
    while (*word_boundary != ' ' && *word_boundary != '?' && word_boundary < end) {
        ++word_boundary;
    }
    protoRequest->path = QString::fromLatin1(ptr, word_boundary - ptr);

    if (*word_boundary == '?') {
        ptr = word_boundary + 1;
        while (*word_boundary != ' ' && word_boundary < end) {
            ++word_boundary;
        }
        protoRequest->query = QByteArray(ptr, word_boundary - ptr);
    } else {
        protoRequest->query = QByteArray();
    }

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    protoRequest->protocol = QString::fromLatin1(ptr, word_boundary - ptr);
}


inline QString normalizeHeaderKey(const char *str, int size)
{
    int i = 0;
    QString key = QString::fromLatin1(str, size);
    while (i < key.size()) {
        QCharRef c = key[i];
        if (c.isLetter()) {
            if (c.isLower()) {
                c = c.toUpper();
            }
        } else if (c == QLatin1Char('-')) {
            c = QLatin1Char('_');
        }
        ++i;
    }
    return key;
}

void ProtocolHttp::parseHeader(const char *ptr, const char *end, Socket *sock) const
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    const char *word_boundary = ptr;
    while (*word_boundary != ':' && word_boundary < end) {
        ++word_boundary;
    }
    const QString key = normalizeHeaderKey(ptr, word_boundary - ptr);

    while ((*word_boundary == ':' || *word_boundary == ' ') && word_boundary < end) {
        ++word_boundary;
    }
    const QString value = QString::fromLatin1(word_boundary, end - word_boundary);

    if (protoRequest->headerConnection == ProtoRequestHttp::HeaderConnectionNotSet && key == QLatin1String("CONNECTION")) {
        if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionClose;
        } else {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnectionKeep;
        }
    } else if (protoRequest->contentLength < 0 && key == QLatin1String("CONTENT_LENGTH")) {
        bool ok;
        qint64 cl = value.toLongLong(&ok);
        if (ok && cl >= 0) {
            protoRequest->contentLength = cl;
        }
    } else if (!protoRequest->headerHost && key == QLatin1String("HOST")) {
        protoRequest->serverAddress = value;
        protoRequest->headerHost = true;
    }
    protoRequest->headers.pushRawHeader(key, value);
}

ProtoRequestHttp::ProtoRequestHttp(WSGI *wsgi, Cutelyst::Engine *_engine)
{

}

ProtoRequestHttp::~ProtoRequestHttp()
{

}

qint64 ProtoRequestHttp::doWrite(const char *data, qint64 len)
{
    qint64 ret = sock->proto->sendBody(io, sock, data, len);
    return ret;
}

bool ProtoRequestHttp::webSocketSendTextMessage(const QString &message)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = message.toUtf8();
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(ProtoRequestHttp::OpCodeText, rawMessage.size());
    doWrite(headers);
    return doWrite(rawMessage) == rawMessage.size();
}

bool ProtoRequestHttp::webSocketSendBinaryMessage(const QByteArray &message)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(ProtoRequestHttp::OpCodeBinary, message.size());
    doWrite(headers);
    return doWrite(message) == message.size();
}

bool ProtoRequestHttp::webSocketSendPing(const QByteArray &payload)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = payload.left(125);
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(ProtoRequestHttp::OpCodePing, rawMessage.size());
    doWrite(headers);
    return doWrite(rawMessage) == rawMessage.size();
}

bool ProtoRequestHttp::webSocketClose(quint16 code, const QString &reason)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray reply = ProtocolWebSocket::createWebsocketCloseReply(reason, code);
    return doWrite(reply) == reply.size();
}

bool ProtoRequestHttp::webSocketHandshakeDo(Cutelyst::Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    if (headerConnection == ProtoRequestHttp::HeaderConnectionUpgrade) {
        return true;
    }

    if (sock->proto->type() != Protocol::Http11) {
        qCWarning(CWSGI_SOCK) << "Upgrading a connection to websocket is only supported with the HTTP/1.1 protocol" << typeid(sock->proto).name();
        return false;
    }

    const Cutelyst::Headers requestHeaders = c->request()->headers();
    Cutelyst::Response *response = c->response();
    Cutelyst::Headers &headers = response->headers();

    response->setStatus(Cutelyst::Response::SwitchingProtocols);
    headers.setHeader(QStringLiteral("UPGRADE"), QStringLiteral("WebSocket"));
    headers.setHeader(QStringLiteral("CONNECTION"), QStringLiteral("Upgrade"));
    const QString localOrigin = origin.isEmpty() ? requestHeaders.header(QStringLiteral("ORIGIN")) : origin;
    headers.setHeader(QStringLiteral("SEC_WEBSOCKET_ORIGIN"), localOrigin.isEmpty() ? QStringLiteral("*") : localOrigin);

    const QString wsProtocol = protocol.isEmpty() ? requestHeaders.header(QStringLiteral("SEC_WEBSOCKET_PROTOCOL")) : protocol;
    if (!wsProtocol.isEmpty()) {
        headers.setHeader(QStringLiteral("SEC_WEBSOCKET_PROTOCOL"), wsProtocol);
    }

    const QString localKey = key.isEmpty() ? requestHeaders.header(QStringLiteral("SEC_WEBSOCKET_KEY")) : key;
    const QString wsKey = localKey + QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    if (wsKey.length() == 36) {
        qCWarning(CWSGI_SOCK) << "Missing websocket key";
        return false;
    }

    const QByteArray wsAccept = QCryptographicHash::hash(wsKey.toLatin1(), QCryptographicHash::Sha1).toBase64();
    headers.setHeader(QStringLiteral("SEC_WEBSOCKET_ACCEPT"), QString::fromLatin1(wsAccept));

    headerConnection = ProtoRequestHttp::HeaderConnectionUpgrade;
    websocketContext = c;

    return writeHeaders(Cutelyst::Response::SwitchingProtocols, headers);
}
