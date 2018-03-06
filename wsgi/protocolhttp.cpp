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
    if (sock->protoRequest->connState == ProtoRequest::ContentBody) {
        qint64 bytesAvailable = io->bytesAvailable();
        int len;
        qint64 remaining;

        QIODevice *body = sock->protoRequest->body;
        do {
            remaining = sock->protoRequest->contentLength - body->size();
            len = io->read(m_postBuffer, qMin(m_postBufferSize, remaining));
            if (len == -1) {
                sock->connectionClose();
                return;
            }
            bytesAvailable -= len;
//            qCDebug(CWSGI_HTTP) << "WRITE body" << sock->protoRequest->contentLength << remaining << len << (remaining == len) << sock->protoRequest->bytesAvailable();
            body->write(m_postBuffer, len);
        } while (bytesAvailable);

        if (remaining == len) {
            processRequest(sock);
        }

        return;
    }

    int len = io->read(sock->protoRequest->buffer + sock->protoRequest->buf_size, m_bufferSize - sock->protoRequest->buf_size);
    if (len == -1) {
        qCWarning(CWSGI_HTTP) << "Failed to read from socket" << io->errorString();
        return;
    }
    sock->protoRequest->buf_size += len;

    while (sock->protoRequest->last < sock->protoRequest->buf_size) {
//        qCDebug(CWSGI_HTTP) << Q_FUNC_INFO << QByteArray(sock->protoRequest->buf, sock->protoRequest->buf_size);
        int ix = CrLfIndexIn(sock->protoRequest->buffer, sock->protoRequest->buf_size, sock->protoRequest->last);
        if (ix != -1) {
            int len = ix - sock->protoRequest->beginLine;
            char *ptr = sock->protoRequest->buffer + sock->protoRequest->beginLine;
            sock->protoRequest->beginLine = ix + 2;
            sock->protoRequest->last = sock->protoRequest->beginLine;

            if (sock->protoRequest->connState == ProtoRequest::MethodLine) {
                if (!sock->protoRequest->startOfRequest) {
                    sock->protoRequest->startOfRequest = sock->protoRequest->engine->time();
                }
                parseMethod(ptr, ptr + len, sock);
                sock->protoRequest->connState = ProtoRequest::HeaderLine;
                sock->protoRequest->contentLength = -1;
                sock->protoRequest->headers = Cutelyst::Headers();
//                qCDebug(CWSGI_HTTP) << "--------" << sock->protoRequest->method << sock->protoRequest->path << sock->protoRequest->query << sock->protoRequest->protocol;

            } else if (sock->protoRequest->connState == ProtoRequest::HeaderLine) {
                if (len) {
                    parseHeader(ptr, ptr + len, sock);
                } else {
                    if (sock->protoRequest->contentLength != -1) {
                        sock->protoRequest->connState = ProtoRequest::ContentBody;
                        if (m_postBuffering && sock->protoRequest->contentLength > m_postBuffering) {
                            auto temp = new QTemporaryFile;
                            if (!temp->open()) {
                                qCWarning(CWSGI_HTTP) << "Failed to open temporary file to store post" << temp->errorString();
                                io->close(); // On error close immediately
                                return;
                            }
                            sock->protoRequest->body = temp;
                        } else if (m_postBuffering && sock->protoRequest->contentLength <= m_postBuffering) {
                            auto buffer = new QBuffer;
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(sock->protoRequest->contentLength);
                            sock->protoRequest->body = buffer;
                        } else {
                            // Unbuffered
                            auto buffer = new QBuffer;
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(sock->protoRequest->contentLength);
                            sock->protoRequest->body = buffer;
                        }

                        ptr += 2;
                        len = qMin(sock->protoRequest->contentLength, static_cast<qint64>(sock->protoRequest->buf_size - sock->protoRequest->last));
//                        qCDebug(CWSGI_HTTP) << "WRITE" << sock->protoRequest->contentLength << len;
                        if (len) {
                            sock->protoRequest->body->write(ptr, len);
                        }
                        sock->protoRequest->last += len;

                        if (sock->protoRequest->contentLength > len) {
//                            qCDebug(CWSGI_HTTP) << "WRITE more..." << sock->protoRequest->contentLength << len;
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
            if (!sock->protoRequest->startOfRequest) {
                sock->protoRequest->startOfRequest = sock->protoRequest->engine->time();
            }
            sock->protoRequest->last = sock->protoRequest->buf_size;
        }
    }

    if (sock->protoRequest->buf_size == m_bufferSize) {
        // 414 Request-URI Too Long
    }
}

bool ProtocolHttp::sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers)
{
    int msgLen;
    const char *msg = CWsgiEngine::httpStatusMessage(status, &msgLen);
    io->write(msg, msgLen);

    const auto headersData = headers.data();
    ProtoRequest::HeaderConnection fallbackConnection = sock->protoRequest->headerConnection;
    sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionNotSet;

    bool hasDate = false;
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
        const QString &key = it.key();
        const QString &value = it.value();
        if (sock->protoRequest->headerConnection == ProtoRequest::HeaderConnectionNotSet && key == QLatin1String("CONNECTION")) {
            if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
                sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionClose;
            } else if (value.compare(QLatin1String("upgrade"), Qt::CaseInsensitive) == 0) {
                sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionUpgrade;
            } else {
                sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionKeep;
            }
        } else if (!hasDate && key == QLatin1String("DATE")) {
            hasDate = true;
        }

        QString ret(QLatin1String("\r\n") + CWsgiEngine::camelCaseHeader(key) + QLatin1String(": ") + value);
        io->write(ret.toLatin1());

        ++it;
    }

    if (sock->protoRequest->headerConnection == ProtoRequest::HeaderConnectionNotSet) {
        if (fallbackConnection == ProtoRequest::HeaderConnectionKeep) {
            sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionKeep;
            io->write("\r\nConnection: keep-alive", 24);
        } else {
            sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionClose;
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
//    qCDebug(CWSGI_HTTP) << "processRequest" << sock->protoRequest->contentLength;
    sock->protoRequest->processing = true;
    if (sock->protoRequest->body) {
        sock->protoRequest->body->seek(0);
    }

    Cutelyst::Context *c = sock->protoRequest->engine->processRequest(sock->protoRequest);
    sock->protoRequest->processing = false;

    if (sock->protoRequest->headerConnection == ProtoRequest::HeaderConnectionUpgrade) {
        // need 2 byte header
        sock->protoRequest->websocket_need = 2;
        sock->protoRequest->websocket_phase = ProtoRequest::WebSocketPhaseHeaders;
        sock->protoRequest->processing = true;
        sock->protoRequest->buf_size = 0;
        sock->protoRequest->proto = m_websocketProto;

        return false; // Must read remaining data
    }
    delete c;

    if (sock->protoRequest->headerConnection == ProtoRequest::HeaderConnectionClose) {
        sock->connectionClose();
        return false;
    }

    if (sock->protoRequest->last < sock->protoRequest->buf_size) {
        // move pipelined request to 0
        int remaining = sock->protoRequest->buf_size - sock->protoRequest->last;
        memmove(sock->protoRequest->buffer, sock->protoRequest->buffer + sock->protoRequest->last, remaining);
        sock->resetSocket();
        sock->protoRequest->buf_size = remaining;

        QCoreApplication::processEvents();
    } else {
        sock->resetSocket();
    }

    return true;
}

void ProtocolHttp::parseMethod(const char *ptr, const char *end, Socket *sock) const
{
    const char *word_boundary = ptr;
    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    sock->protoRequest->method = QString::fromLatin1(ptr, word_boundary - ptr);

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
    sock->protoRequest->path = QString::fromLatin1(ptr, word_boundary - ptr);

    if (*word_boundary == '?') {
        ptr = word_boundary + 1;
        while (*word_boundary != ' ' && word_boundary < end) {
            ++word_boundary;
        }
        sock->protoRequest->query = QByteArray(ptr, word_boundary - ptr);
    } else {
        sock->protoRequest->query = QByteArray();
    }

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    sock->protoRequest->protocol = QString::fromLatin1(ptr, word_boundary - ptr);
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
    const char *word_boundary = ptr;
    while (*word_boundary != ':' && word_boundary < end) {
        ++word_boundary;
    }
    const QString key = normalizeHeaderKey(ptr, word_boundary - ptr);

    while ((*word_boundary == ':' || *word_boundary == ' ') && word_boundary < end) {
        ++word_boundary;
    }
    const QString value = QString::fromLatin1(word_boundary, end - word_boundary);

    if (sock->protoRequest->headerConnection == ProtoRequest::HeaderConnectionNotSet && key == QLatin1String("CONNECTION")) {
        if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
            sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionClose;
        } else {
            sock->protoRequest->headerConnection = ProtoRequest::HeaderConnectionKeep;
        }
    } else if (sock->protoRequest->contentLength < 0 && key == QLatin1String("CONTENT_LENGTH")) {
        bool ok;
        qint64 cl = value.toLongLong(&ok);
        if (ok && cl >= 0) {
            sock->protoRequest->contentLength = cl;
        }
    } else if (!sock->protoRequest->headerHost && key == QLatin1String("HOST")) {
        sock->protoRequest->serverAddress = value;
        sock->protoRequest->headerHost = true;
    }
    sock->protoRequest->headers.pushRawHeader(key, value);
}
