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
#include "tcpserver.h"
#include "localserver.h"
#include "config.h"
#include "wsgi.h"
#include "staticmap.h"
#include "socket.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimer>

using namespace CWSGI;
using namespace Cutelyst;

QByteArray dateHeader();

CWsgiEngine::CWsgiEngine(Application *app, int workerCore, const QVariantMap &opts, WSGI *wsgi) : Engine(app, workerCore, opts)
  , m_wsgi(wsgi)
{
    defaultHeaders().setServer(QLatin1String("cutelyst/") + QLatin1String(VERSION));

    m_lastDate = dateHeader();
    m_lastDateTimer.start();

    const QString staticMap = wsgi->staticMap();
    const QString staticMap2 = wsgi->staticMap2();
    if (!staticMap.isEmpty() || !staticMap2.isEmpty()) {
        auto staticMapPlugin = new StaticMap(app);

        const auto parts = staticMap.split(QLatin1Char(';'));
        for (const QString &part : parts) {
            staticMapPlugin->addStaticMap(part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), false);
        }

        const auto parts2 = staticMap2.split(QLatin1Char(';'));
        for (const QString &part : parts2) {
            staticMapPlugin->addStaticMap(part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), true);
        }
    }

    if (wsgi->socketTimeout()) {
        m_socketTimeout = new QTimer(this);
        m_socketTimeout->setInterval(wsgi->socketTimeout() * 1000);
    }
}

int CWsgiEngine::workerId() const
{
    return m_workerId;
}

void CWsgiEngine::setTcpSockets(const std::vector<SocketInfo> &sockets)
{
    m_sockets = sockets;
}

void CWsgiEngine::listen()
{
    if (workerCore() > 0) {
        // init and postfork
        if (!initApplication()) {
            qCritical() << "Failed to init application on a different thread than main. Are you sure threaded mode is supported in this application?";
            return;
        }
    }

    const auto sockets = m_sockets;
    for (const SocketInfo &info : sockets) {
        if (!info.localSocket) {
            auto server = new TcpServer(info.serverName, info.protocol, m_wsgi, this);
            if (server->setSocketDescriptor(info.socketDescriptor)) {
                server->pauseAccepting();
                connect(this, &CWsgiEngine::started, server, &TcpServer::resumeAccepting);
                connect(this, &CWsgiEngine::shutdown, server, &TcpServer::shutdown);
                connect(server, &TcpServer::shutdownCompleted, this, &CWsgiEngine::serverShutdown);
                if (m_socketTimeout) {
                    connect(server, &TcpServer::startSocketTimeout, this, &CWsgiEngine::startSocketTimeout);
                    connect(server, &TcpServer::stopSocketTimeout, this, &CWsgiEngine::stopSocketTimeout);
                    connect(m_socketTimeout, &QTimer::timeout, server, &TcpServer::timeoutConnections);
                }
            }
        } else {
            auto server = new LocalServer(QStringLiteral("localhost"), info.protocol, m_wsgi, this);
            if (server->setSocketDescriptor(info.socketDescriptor)) {
                server->pauseAccepting();
                connect(this, &CWsgiEngine::started, server, &LocalServer::resumeAccepting);
                connect(this, &CWsgiEngine::shutdown, server, &LocalServer::shutdown);
                connect(server, &LocalServer::shutdownCompleted, this, &CWsgiEngine::serverShutdown);
                if (m_socketTimeout) {
                    connect(server, &LocalServer::startSocketTimeout, this, &CWsgiEngine::startSocketTimeout);
                    connect(server, &LocalServer::stopSocketTimeout, this, &CWsgiEngine::stopSocketTimeout);
                    connect(m_socketTimeout, &QTimer::timeout, server, &LocalServer::timeoutConnections);
                }
            }
        }
        ++m_servers;
    }

    Q_EMIT initted();
}

void CWsgiEngine::postFork()
{
    if (!postForkApplication()) {
        // CHEAP
        QCoreApplication::exit(15);
    }

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

void CWsgiEngine::serverShutdown()
{
    if (--m_servers == 0) {
        Q_EMIT shutdownCompleted(this);
    }
}

void CWsgiEngine::startSocketTimeout()
{
    if (++m_serversTimeout == 1) {
        m_socketTimeout->start();
    }
}

void CWsgiEngine::stopSocketTimeout()
{
    if (--m_serversTimeout == 0) {
        m_socketTimeout->stop();
    }
}

bool CWsgiEngine::init()
{
    return initApplication();
}

#include "moc_cwsgiengine.cpp"
