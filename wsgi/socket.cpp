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
#include "socket.h"

#include "wsgi.h"
#include "protocolwebsocket.h"

#include <typeinfo>

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QCoreApplication>
#include <QDebug>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_SOCK, "cwsgi.socket")

using namespace CWSGI;

TcpSocket::TcpSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QTcpSocket(parent), Socket(wsgi, engine)
{
    io = this;
    isSecure = false;
    startOfRequest = 0;
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected, Qt::DirectConnection);
}

void TcpSocket::connectionClose()
{
    flush();
    disconnectFromHost();
}

Socket::Socket(WSGI *wsgi, Cutelyst::Engine *_engine) : EngineRequest(_engine)
{
    body = nullptr;
    buffer = new char[wsgi->bufferSize()];
}

Socket::~Socket()
{
    delete [] buffer;
}

bool Socket::webSocketSendTextMessage(const QString &message)
{
    if (headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = message.toUtf8();
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodeText, rawMessage.size());
    doWrite(headers);
    return doWrite(rawMessage) == rawMessage.size();
}

bool Socket::webSocketSendBinaryMessage(const QByteArray &message)
{
    if (headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodeBinary, message.size());
    doWrite(headers);
    return doWrite(message) == message.size();
}

bool Socket::webSocketSendPing(const QByteArray &payload)
{
    if (headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = payload.left(125);
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodePing, rawMessage.size());
    doWrite(headers);
    return doWrite(rawMessage) == rawMessage.size();
}

bool Socket::webSocketClose(quint16 code, const QString &reason)
{
    if (headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray reply = ProtocolWebSocket::createWebsocketCloseReply(reason, code);
    return doWrite(reply) == reply.size();
}

qint64 Socket::doWrite(const char *data, qint64 len)
{
    qint64 ret = proto->sendBody(io, this, data, len);
    return ret;
}

bool Socket::writeHeaders(quint16 status, const Cutelyst::Headers &headers)
{
    return proto->sendHeaders(io, this, status, static_cast<CWsgiEngine *>(engine)->lastDate(), headers);
}

bool Socket::webSocketHandshakeDo(Cutelyst::Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    if (headerConnection == Socket::HeaderConnectionUpgrade) {
        return true;
    }

    if (proto->type() != Protocol::Http11) {
        qCWarning(CWSGI_SOCK) << "Upgrading a connection to websocket is only supported with the HTTP protocol" << typeid(proto).name();
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

    headerConnection = Socket::HeaderConnectionUpgrade;
    websocketContext = c;

    return writeHeaders(Cutelyst::Response::SwitchingProtocols, headers);
}

void TcpSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

LocalSocket::LocalSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QLocalSocket(parent), Socket(wsgi, engine)
{
    io = this;
    isSecure = false;
    startOfRequest = 0;
    connect(this, &QLocalSocket::disconnected, this, &LocalSocket::socketDisconnected, Qt::DirectConnection);
}

void LocalSocket::connectionClose()
{
    flush();
    disconnectFromServer();
}

void LocalSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

SslSocket::SslSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QSslSocket(parent), Socket(wsgi, engine)
{
    io = this;
    isSecure = true;
    startOfRequest = 0;
    connect(this, &QSslSocket::disconnected, this, &SslSocket::socketDisconnected, Qt::DirectConnection);
}

void SslSocket::connectionClose()
{
    flush();
    disconnectFromHost();
}

void SslSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

#include "moc_socket.cpp"
