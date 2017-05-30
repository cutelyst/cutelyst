/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "cwsgiengine.h"

#include "protocol.h"
#include "tcpserverbalancer.h"
#include "tcpserver.h"
#include "tcpsslserver.h"
#include "localserver.h"
#include "config.h"
#include "wsgi.h"
#include "staticmap.h"
#include "socket.h"

#include "protocolwebsocket.h"
#include "protocolhttp.h"
#include "protocolfastcgi.h"

#ifdef Q_OS_UNIX
#include "unixfork.h"
#endif

#include <typeinfo>

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QCoreApplication>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_ENGINE, "cwsgi.engine")

using namespace CWSGI;
using namespace Cutelyst;

QByteArray dateHeader();

CWsgiEngine::CWsgiEngine(Application *localApp, int workerCore, const QVariantMap &opts, WSGI *wsgi) : Engine(localApp, workerCore, opts)
  , m_wsgi(wsgi)
{
    defaultHeaders().setServer(QLatin1String("cutelyst/") + QLatin1String(VERSION));

    m_lastDate = dateHeader();
    m_lastDateTimer.start();

    const QStringList staticMap = m_wsgi->staticMap();
    const QStringList staticMap2 = m_wsgi->staticMap2();
    if (!staticMap.isEmpty() || !staticMap2.isEmpty()) {
        auto staticMapPlugin = new StaticMap(app());

        for (const QString &part : staticMap) {
            staticMapPlugin->addStaticMap(part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), false);
        }

        for (const QString &part : staticMap2) {
            staticMapPlugin->addStaticMap(part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), true);
        }
    }

    if (m_wsgi->socketTimeout()) {
        m_socketTimeout = new QTimer(this);
        m_socketTimeout->setInterval(m_wsgi->socketTimeout() * 1000);
    }
}

int CWsgiEngine::workerId() const
{
    return m_workerId;
}

void CWsgiEngine::setServers(const std::vector<QObject *> &servers)
{
    for (QObject *server : servers) {
        auto balancer = qobject_cast<TcpServerBalancer *>(server);
        if (balancer) {
            TcpServer *server = balancer->createServer(this);
            if (server) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(m_socketTimeout, &QTimer::timeout, server, &TcpServer::timeoutConnections);
                }

                if (server->protocol()->type() == Protocol::Http11) {
                    if (!m_protoHttp) {
                        m_protoHttp = new ProtocolHttp(m_wsgi);
                    }
                    server->setProtocol(m_protoHttp);
                } else if (server->protocol()->type() == Protocol::FastCGI1) {
                    if (!m_protoFcgi) {
                        m_protoFcgi = new ProtocolFastCGI(m_wsgi);
                    }
                    server->setProtocol(m_protoFcgi);
                }
            }
        }

        auto localServer = qobject_cast<LocalServer *>(server);
        if (localServer) {
            LocalServer *server = localServer->createServer(this);
            if (server) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(m_socketTimeout, &QTimer::timeout, server, &LocalServer::timeoutConnections);
                }

                if (server->protocol()->type() == Protocol::Http11) {
                    if (!m_protoHttp) {
                        m_protoHttp = new ProtocolHttp(m_wsgi);
                    }
                    server->setProtocol(m_protoHttp);
                } else if (server->protocol()->type() == Protocol::FastCGI1) {
                    if (!m_protoFcgi) {
                        m_protoFcgi = new ProtocolFastCGI(m_wsgi);
                    }
                    server->setProtocol(m_protoFcgi);
                }
            }
        }
    }
}

void CWsgiEngine::postFork(int workerId)
{
    m_workerId = workerId;

    if (!postForkApplication()) {
        // CHEAP
        Q_EMIT shutdown();
        return;
    }

#ifdef Q_OS_UNIX
    UnixFork::setSched(m_wsgi, workerId, workerCore());
#endif

    Q_EMIT started();
}

QByteArray dateHeader()
{
    QString ret;
    ret = QLatin1String("\r\nDate: ") + QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
    return ret.toLatin1();
}

bool CWsgiEngine::finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData)
{
    auto sock = static_cast<TcpSocket*>(engineData);
    if (sock) {
        if (m_lastDateTimer.hasExpired(1000)) {
            m_lastDate = dateHeader();
            m_lastDateTimer.restart();
        }

        return sock->proto->sendHeaders(sock, sock, status, m_lastDate, headers);
    }
    return false;
}

qint64 CWsgiEngine::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    auto sock = static_cast<TcpSocket*>(engineData);
    auto io = static_cast<QIODevice*>(engineData);
    //    qDebug() << Q_FUNC_INFO << QByteArray(data,len);
    qint64 ret = sock->proto->sendBody(io, sock, data, len);
    //    conn->waitForBytesWritten(200);
    return ret;
}

bool CWsgiEngine::webSocketHandshakeDo(Context *c, const QString &key, const QString &origin, const QString &protocol, void *engineData)
{
    auto sock = static_cast<TcpSocket*>(engineData);
    if (sock->headerConnection == Socket::HeaderConnectionUpgrade) {
        return true;
    }

    if (sock->proto->type() != Protocol::Http11) {
        qCWarning(CWSGI_ENGINE) << "Upgrading a connection to websocket is only supported with the HTTP protocol" << typeid(sock->proto).name();
        return false;
    }

    const Headers requestHeaders = c->request()->headers();
    Response *response = c->response();
    Headers &headers = response->headers();

    response->setStatus(Response::SwitchingProtocols);
    headers.setHeader(QStringLiteral("UPGRADE"), QStringLiteral("WebSocket"));
    headers.setHeader(QStringLiteral("CONNECTION"), QStringLiteral("Upgrade"));
    const QString localOrigin = origin.isEmpty() ? requestHeaders.header(QStringLiteral("ORIGIN")) : origin;
    if (localOrigin.isEmpty()) {
        headers.setHeader(QStringLiteral("SEC_WEBSOCKET_ORIGIN"), QStringLiteral("*"));
    } else {
        headers.setHeader(QStringLiteral("SEC_WEBSOCKET_ORIGIN"), localOrigin);
    }

    const QString wsProtocol = protocol.isEmpty() ? requestHeaders.header(QStringLiteral("SEC_WEBSOCKET_PROTOCOL")) : protocol;
    if (!wsProtocol.isEmpty()) {
        headers.setHeader(QStringLiteral("SEC_WEBSOCKET_PROTOCOL"), wsProtocol);
    }

    const QString localKey = key.isEmpty() ? requestHeaders.header(QStringLiteral("SEC_WEBSOCKET_KEY")) : key;
    const QString wsKey = localKey + QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    if (wsKey.length() == 36) {
        qCWarning(CWSGI_ENGINE) << "Missing websocket key";
        return false;
    }

    const QByteArray wsAccept = QCryptographicHash::hash(wsKey.toLatin1(), QCryptographicHash::Sha1).toBase64();
    headers.setHeader(QStringLiteral("SEC_WEBSOCKET_ACCEPT"), QString::fromLatin1(wsAccept));

    sock->headerConnection = Socket::HeaderConnectionUpgrade;
    sock->websocketContext = c;

    return finalizeHeadersWrite(c, Response::SwitchingProtocols, headers, engineData);
}

bool CWsgiEngine::webSocketSendTextMessage(Context *c, const QString &message)
{
    auto sock = static_cast<TcpSocket*>(c->engineData());
    if (sock->headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = message.toUtf8();
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodeText, rawMessage.size());
    doWrite(c, headers.data(), headers.size(), sock);
    return doWrite(c, rawMessage.data(), rawMessage.size(), sock) == rawMessage.size();
}

bool CWsgiEngine::webSocketSendBinaryMessage(Context *c, const QByteArray &message)
{
    auto sock = static_cast<TcpSocket*>(c->engineData());
    if (sock->headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodeBinary, message.size());
    doWrite(c, headers.data(), headers.size(), sock);
    return doWrite(c, message.data(), message.size(), sock) == message.size();
}

bool CWsgiEngine::webSocketSendPing(Context *c, const QByteArray &payload)
{
    auto sock = static_cast<TcpSocket*>(c->engineData());
    if (sock->headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray rawMessage = payload.left(125);
    const QByteArray headers = ProtocolWebSocket::createWebsocketHeader(Socket::OpCodePing, rawMessage.size());
    doWrite(c, headers.data(), headers.size(), sock);
    return doWrite(c, rawMessage.data(), rawMessage.size(), sock) == rawMessage.size();
}

bool CWsgiEngine::webSocketClose(Context *c, quint16 code, const QString &reason)
{
    auto sock = static_cast<TcpSocket*>(c->engineData());
    if (sock->headerConnection != Socket::HeaderConnectionUpgrade) {
        return false;
    }

    const QByteArray reply = ProtocolWebSocket::createWebsocketCloseReply(reason, code);
    return doWrite(c, reply.data(), reply.size(), sock) == reply.size();
}

bool CWsgiEngine::init()
{
    if (!initApplication()) {
        qCCritical(CWSGI_ENGINE) << "Failed to init application, cheaping...";
        return false;
    }

    return true;
}

#include "moc_cwsgiengine.cpp"
