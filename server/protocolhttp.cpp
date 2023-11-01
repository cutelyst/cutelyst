/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "protocolhttp.h"

#include "protocolhttp2.h"
#include "protocolwebsocket.h"
#include "server.h"
#include "socket.h"

#include <Cutelyst/Context>
#include <Cutelyst/Headers>
#include <Cutelyst/Response>
#include <typeinfo>

#include <QBuffer>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QIODevice>
#include <QLoggingCategory>
#include <QVariant>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(CWSGI_HTTP, "cwsgi.http", QtWarningMsg)
Q_DECLARE_LOGGING_CATEGORY(CWSGI_SOCK)

ProtocolHttp::ProtocolHttp(Server *wsgi, ProtocolHttp2 *upgradeH2c)
    : Protocol(wsgi)
    , m_websocketProto(new ProtocolWebSocket(wsgi))
    , m_upgradeH2c(upgradeH2c)
{
    usingFrontendProxy = wsgi->usingFrontendProxy();
}

ProtocolHttp::~ProtocolHttp()
{
    delete m_websocketProto;
}

Protocol::Type ProtocolHttp::type() const
{
    return Protocol::Type::Http11;
}

inline int CrLfIndexIn(const char *str, int len, int from)
{
    do {
        const char *pch = static_cast<const char *>(memchr(str + from, '\r', size_t(len - from)));
        if (pch != nullptr) {
            int pos = int(pch - str);
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

void ProtocolHttp::parse(Socket *sock, QIODevice *io) const
{
    // Post buffering
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    if (protoRequest->status & Cutelyst::EngineRequest::Async) {
        return;
    }

    if (protoRequest->connState == ProtoRequestHttp::ContentBody) {
        qint64 bytesAvailable = io->bytesAvailable();
        qint64 len;
        qint64 remaining;

        QIODevice *body = protoRequest->body;
        do {
            remaining = protoRequest->contentLength - body->size();
            len       = io->read(m_postBuffer, qMin(m_postBufferSize, remaining));
            if (len == -1) {
                qCWarning(CWSGI_HTTP) << "error while reading body" << len << protoRequest->headers;
                sock->connectionClose();
                return;
            }
            bytesAvailable -= len;
            //            qCDebug(CWSGI_HTTP) << "WRITE body" << protoRequest->contentLength <<
            //            remaining << len << (remaining == len) << io->bytesAvailable();
            body->write(m_postBuffer, len);
        } while (bytesAvailable && remaining);

        if (remaining == len) {
            processRequest(sock, io);
        }

        return;
    }

    qint64 len = io->read(protoRequest->buffer + protoRequest->buf_size,
                          m_bufferSize - protoRequest->buf_size);
    if (len == -1) {
        qCWarning(CWSGI_HTTP) << "Failed to read from socket" << io->errorString();
        return;
    }
    protoRequest->buf_size += len;

    while (protoRequest->last < protoRequest->buf_size) {
        //        qCDebug(CWSGI_HTTP) << Q_FUNC_INFO << QByteArray(protoRequest->buffer,
        //        protoRequest->buf_size);
        int ix = CrLfIndexIn(protoRequest->buffer, protoRequest->buf_size, protoRequest->last);
        if (ix != -1) {
            qint64 len              = ix - protoRequest->beginLine;
            char *ptr               = protoRequest->buffer + protoRequest->beginLine;
            protoRequest->beginLine = ix + 2;
            protoRequest->last      = protoRequest->beginLine;

            if (protoRequest->connState == ProtoRequestHttp::MethodLine) {
                if (useStats && protoRequest->startOfRequest == TimePointSteady{}) {
                    protoRequest->startOfRequest = std::chrono::steady_clock::now();
                }

                parseMethod(ptr, ptr + len, sock);
                protoRequest->connState     = ProtoRequestHttp::HeaderLine;
                protoRequest->contentLength = -1;
                protoRequest->headers       = Cutelyst::Headers();
                //                qCDebug(CWSGI_HTTP) << "--------" << protoRequest->method <<
                //                protoRequest->path << protoRequest->query <<
                //                protoRequest->protocol;

            } else if (protoRequest->connState == ProtoRequestHttp::HeaderLine) {
                if (len) {
                    parseHeader(ptr, ptr + len, sock);
                } else {
                    if (protoRequest->contentLength > 0) {
                        protoRequest->connState = ProtoRequestHttp::ContentBody;
                        protoRequest->body      = createBody(protoRequest->contentLength);
                        if (!protoRequest->body) {
                            qCWarning(CWSGI_HTTP) << "error while creating body, closing socket";
                            sock->connectionClose();
                            return;
                        }

                        ptr += 2;
                        len =
                            qMin(protoRequest->contentLength,
                                 static_cast<qint64>(protoRequest->buf_size - protoRequest->last));
                        //                        qCDebug(CWSGI_HTTP) << "WRITE" <<
                        //                        protoRequest->contentLength << len;
                        if (len) {
                            protoRequest->body->write(ptr, len);
                        }
                        protoRequest->last += len;

                        if (protoRequest->contentLength > len) {
                            //                            qCDebug(CWSGI_HTTP) << "WRITE more..." <<
                            //                            protoRequest->contentLength << len;
                            // body is not completed yet
                            if (io->bytesAvailable()) {
                                // since we still have bytes available call this function
                                // so that the body parser reads the rest of available data
                                parse(sock, io);
                            }
                            return;
                        }
                    }

                    if (!processRequest(sock, io)) {
                        break;
                    }
                }
            }
        } else {
            if (protoRequest->startOfRequest == TimePointSteady{}) {
                protoRequest->startOfRequest = std::chrono::steady_clock::now();
            }
            protoRequest->last = protoRequest->buf_size;
        }
    }

    if (protoRequest->buf_size == m_bufferSize) {
        // 414 Request-URI Too Long
    }
}

ProtocolData *ProtocolHttp::createData(Socket *sock) const
{
    return new ProtoRequestHttp(sock, m_bufferSize);
}

bool ProtocolHttp::processRequest(Socket *sock, QIODevice *io) const
{
    auto request = static_cast<ProtoRequestHttp *>(sock->protoData);
    //    qCDebug(CWSGI_HTTP) << "processRequest" << sock->protoData->contentLength;
    if (request->body) {
        request->body->seek(0);
    }

    // When enabled try to upgrade to H2C
    if (m_upgradeH2c && m_upgradeH2c->upgradeH2C(sock, io, *request)) {
        return false;
    }

    ++sock->processing;
    sock->engine->processRequest(request);

    if (request->websocketUpgraded) {
        return false; // Must read remaining data
    }

    if (request->status & Cutelyst::EngineRequest::Async) {
        return false; // Need to break now
    }

    return true;
}

void ProtocolHttp::parseMethod(const char *ptr, const char *end, Socket *sock) const
{
    auto protoRequest         = static_cast<ProtoRequestHttp *>(sock->protoData);
    const char *word_boundary = ptr;
    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    protoRequest->method = QByteArray(ptr, int(word_boundary - ptr));

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    // find path end
    while (*word_boundary != ' ' && *word_boundary != '?' && word_boundary < end) {
        ++word_boundary;
    }

    // This will change the ptr but will only change less than size
    protoRequest->setPath(const_cast<char *>(ptr), int(word_boundary - ptr));

    if (*word_boundary == '?') {
        ptr = word_boundary + 1;
        while (*word_boundary != ' ' && word_boundary < end) {
            ++word_boundary;
        }
        protoRequest->query = QByteArray(ptr, int(word_boundary - ptr));
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
    protoRequest->protocol = QByteArray(ptr, int(word_boundary - ptr));
}

void ProtocolHttp::parseHeader(const char *ptr, const char *end, Socket *sock) const
{
    auto protoRequest         = static_cast<ProtoRequestHttp *>(sock->protoData);
    const char *word_boundary = ptr;
    while (*word_boundary != ':' && word_boundary < end) {
        ++word_boundary;
    }
    const auto key = QByteArray(ptr, int(word_boundary - ptr));

    while ((*word_boundary == ':' || *word_boundary == ' ') && word_boundary < end) {
        ++word_boundary;
    }
    const auto value = QByteArray(word_boundary, int(end - word_boundary));

    if (protoRequest->headerConnection == ProtoRequestHttp::HeaderConnection::NotSet &&
        key.compare("Connection", Qt::CaseInsensitive) == 0) {
        if (value.compare("close", Qt::CaseInsensitive) == 0) {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnection::Close;
        } else {
            protoRequest->headerConnection = ProtoRequestHttp::HeaderConnection::Keep;
        }
    } else if (protoRequest->contentLength < 0 &&
               key.compare("Content-Length", Qt::CaseInsensitive) == 0) {
        bool ok;
        qint64 cl = value.toLongLong(&ok);
        if (ok && cl >= 0) {
            protoRequest->contentLength = cl;
        }
    } else if (!protoRequest->headerHost && key.compare("Host", Qt::CaseInsensitive) == 0) {
        protoRequest->serverAddress = value;
        protoRequest->headerHost    = true;
    } else if (usingFrontendProxy) {
        if (!protoRequest->X_Forwarded_For &&
            (key.compare("X-Forwarded-For", Qt::CaseInsensitive) == 0 ||
             key.compare("X-Real-Ip", Qt::CaseInsensitive) == 0)) {
            // configure your reverse-proxy to list only one IP address
            protoRequest->remoteAddress.setAddress(QString::fromLatin1(value));
            protoRequest->remotePort      = 0; // unknown
            protoRequest->X_Forwarded_For = true;
        } else if (!protoRequest->X_Forwarded_Host &&
                   key.compare("X-Forwarded-Host", Qt::CaseInsensitive) == 0) {
            protoRequest->serverAddress    = value;
            protoRequest->X_Forwarded_Host = true;
            protoRequest->headerHost       = true; // ignore a following Host: header (if any)
        } else if (!protoRequest->X_Forwarded_Proto &&
                   key.compare("X-Forwarded-Proto", Qt::CaseInsensitive) == 0) {
            protoRequest->isSecure          = (value.compare("https") == 0);
            protoRequest->X_Forwarded_Proto = true;
        }
    }
    protoRequest->headers.pushHeader(key, value);
}

ProtoRequestHttp::ProtoRequestHttp(Socket *sock, int bufferSize)
    : ProtocolData(sock, bufferSize)
{
    isSecure = sock->isSecure;
}

ProtoRequestHttp::~ProtoRequestHttp()
{
}

void ProtoRequestHttp::setupNewConnection(Socket *sock)
{
    serverAddress = sock->serverAddress;
    remoteAddress = sock->remoteAddress;
    remotePort    = sock->remotePort;
}

bool ProtoRequestHttp::writeHeaders(quint16 status, const Cutelyst::Headers &headers)
{
    if (websocketUpgraded && status != Cutelyst::Response::SwitchingProtocols) {
        qCWarning(CWSGI_SOCK) << "Trying to write header while on an Websocket context";
        return false;
    }

    int msgLen;
    const char *msg = ServerEngine::httpStatusMessage(status, &msgLen);
    QByteArray data(msg, msgLen);

    const auto headersData                                = headers.data();
    ProtoRequestHttp::HeaderConnection fallbackConnection = headerConnection;
    headerConnection = ProtoRequestHttp::HeaderConnection::NotSet;

    bool hasDate = false;
    auto it      = headersData.begin();
    while (it != headersData.end()) {
        if (headerConnection == ProtoRequestHttp::HeaderConnection::NotSet &&
            it->key.compare("Connection", Qt::CaseInsensitive) == 0) {
            if (it->value.compare("close") == 0) {
                headerConnection = ProtoRequestHttp::HeaderConnection::Close;
            } else if (it->value.compare("Upgrade") == 0) {
                headerConnection = ProtoRequestHttp::HeaderConnection::Upgrade;
            } else {
                headerConnection = ProtoRequestHttp::HeaderConnection::Keep;
            }
        } else if (!hasDate && it->key.compare("Date", Qt::CaseInsensitive) == 0) {
            hasDate = true;
        }

        data.append("\r\n");
        data.append(it->key);
        data.append(": ");
        data.append(it->value);

        ++it;
    }

    if (headerConnection == ProtoRequestHttp::HeaderConnection::NotSet) {
        if (fallbackConnection == ProtoRequestHttp::HeaderConnection::Keep ||
            (fallbackConnection != ProtoRequestHttp::HeaderConnection::Close &&
             protocol.compare("HTTP/1.1") == 0)) {
            headerConnection = ProtoRequestHttp::HeaderConnection::Keep;
            data.append("\r\nConnection: keep-alive", 24);
        } else {
            headerConnection = ProtoRequestHttp::HeaderConnection::Close;
            data.append("\r\nConnection: close", 19);
        }
    }

    if (!hasDate) {
        data.append(static_cast<ServerEngine *>(sock->engine)->lastDate());
    }
    data.append("\r\n\r\n", 4);

    return io->write(data) == data.size();
}

qint64 ProtoRequestHttp::doWrite(const char *data, qint64 len)
{
    return io->write(data, len);
}

void ProtoRequestHttp::processingFinished()
{
    if (websocketUpgraded) {
        // need 2 byte header
        websocket_need  = 2;
        websocket_phase = ProtoRequestHttp::WebSocketPhaseHeaders;
        buf_size        = 0;
        return;
    }

    if (!sock->requestFinished()) {
        // disconnected
        return;
    }

    if (headerConnection == ProtoRequestHttp::HeaderConnection::Close) {
        sock->connectionClose();
        return;
    }

    if (last < buf_size) {
        // move pipelined request to 0
        int remaining = buf_size - last;
        memmove(buffer, buffer + last, size_t(remaining));
        resetData();
        buf_size = remaining;

        if (status & EngineRequest::Async) {
            sock->proto->parse(sock, io);
        }
    } else {
        resetData();
    }
}

bool ProtoRequestHttp::webSocketSendTextMessage(const QString &message)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnection::Upgrade) {
        qCWarning(CWSGI_HTTP)
            << "Not sending websocket text message due connection header not upgraded"
            << headerConnection << message.size();
        return false;
    }

    const QByteArray rawMessage = message.toUtf8();
    const QByteArray headers    = ProtocolWebSocket::createWebsocketHeader(
        ProtoRequestHttp::OpCodeText, quint64(rawMessage.size()));
    return doWrite(headers) == headers.size() && doWrite(rawMessage) == rawMessage.size();
}

bool ProtoRequestHttp::webSocketSendBinaryMessage(const QByteArray &message)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnection::Upgrade) {
        qCWarning(CWSGI_HTTP)
            << "Not sending websocket binary messagedue connection header not upgraded"
            << headerConnection << message.size();
        return false;
    }

    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(
        ProtoRequestHttp::OpCodeBinary, quint64(message.size()));
    return doWrite(headers) == headers.size() && doWrite(message) == message.size();
}

bool ProtoRequestHttp::webSocketSendPing(const QByteArray &payload)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnection::Upgrade) {
        qCWarning(CWSGI_HTTP) << "Not sending websocket ping due connection header not upgraded"
                              << headerConnection << payload.size();
        return false;
    }

    const QByteArray rawMessage = payload.left(125);
    const QByteArray headers    = ProtocolWebSocket::createWebsocketHeader(
        ProtoRequestHttp::OpCodePing, quint64(rawMessage.size()));
    return doWrite(headers) == headers.size() && doWrite(rawMessage) == rawMessage.size();
}

bool ProtoRequestHttp::webSocketClose(quint16 code, const QString &reason)
{
    if (headerConnection != ProtoRequestHttp::HeaderConnection::Upgrade) {
        qCWarning(CWSGI_HTTP) << "Not sending websocket close due connection header not upgraded"
                              << headerConnection << code << reason;
        return false;
    }

    const QByteArray reply = ProtocolWebSocket::createWebsocketCloseReply(reason, code);
    bool ret               = doWrite(reply) == reply.size();
    sock->requestFinished();
    sock->connectionClose();
    return ret;
}

void ProtoRequestHttp::socketDisconnected()
{
    if (websocketUpgraded) {
        if (websocket_finn_opcode != 0x88) {
            Q_EMIT context->request()->webSocketClosed(1005, QString{});
        }
        sock->requestFinished();
    }
}

bool ProtoRequestHttp::webSocketHandshakeDo(const QByteArray &key,
                                            const QByteArray &origin,
                                            const QByteArray &protocol)
{
    if (headerConnection == ProtoRequestHttp::HeaderConnection::Upgrade) {
        return true;
    }

    if (sock->proto->type() != Protocol::Type::Http11) {
        qCWarning(CWSGI_SOCK)
            << "Upgrading a connection to websocket is only supported with the HTTP/1.1 protocol"
            << typeid(sock->proto).name();
        return false;
    }

    const Cutelyst::Headers requestHeaders = context->request()->headers();
    Cutelyst::Response *response           = context->response();
    Cutelyst::Headers &headers             = response->headers();

    response->setStatus(Cutelyst::Response::SwitchingProtocols);
    headers.setHeader("Upgrade"_qba, "WebSocket"_qba);
    headers.setHeader("Connection"_qba, "Upgrade"_qba);
    const auto localOrigin = origin.isEmpty() ? requestHeaders.header("Origin") : origin;
    headers.setHeader("Sec-Websocket-Origin"_qba, localOrigin.isEmpty() ? "*"_qba : localOrigin);

    if (!protocol.isEmpty()) {
        headers.setHeader("Sec-Websocket-Protocol"_qba, protocol);
    } else if (const auto wsProtocol = requestHeaders.header("Sec-Websocket-Protocol");
               !wsProtocol.isEmpty()) {
        headers.setHeader("Sec-Websocket-Protocol"_qba, wsProtocol);
    }

    const QByteArray localKey = key.isEmpty() ? requestHeaders.header("Sec-Websocket-Key") : key;
    const QByteArray wsKey    = localKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    if (wsKey.length() == 36) {
        qCWarning(CWSGI_SOCK) << "Missing websocket key";
        return false;
    }

    const QByteArray wsAccept =
        QCryptographicHash::hash(wsKey, QCryptographicHash::Sha1).toBase64();
    headers.setHeader("Sec-Websocket-Accept"_qba, wsAccept);

    headerConnection  = ProtoRequestHttp::HeaderConnection::Upgrade;
    websocketUpgraded = true;
    auto httpProto    = static_cast<ProtocolHttp *>(sock->proto);
    sock->proto       = httpProto->m_websocketProto;

    return writeHeaders(Cutelyst::Response::SwitchingProtocols, headers);
}

#include "moc_protocolhttp.cpp"
