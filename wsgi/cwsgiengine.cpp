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

#ifdef Q_OS_UNIX
#include "unixfork.h"
#endif

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QCoreApplication>

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
                if (m_socketTimeout) {
                    connect(m_socketTimeout, &QTimer::timeout, server, &TcpServer::timeoutConnections);
                }
                m_tcpServers.push_back(server);
            }
        }

        auto localServer = qobject_cast<LocalServer *>(server);
        if (localServer) {
            auto server = new LocalServer(QStringLiteral("localhost"), localServer->protocol(), m_wsgi, this);
            if (server->setSocketDescriptor(localServer->socket())) {
                server->pauseAccepting();
                connect(this, &CWsgiEngine::started, server, &LocalServer::resumeAccepting);
                connect(this, &CWsgiEngine::shutdown, server, &LocalServer::shutdown);
                if (m_socketTimeout) {
                    connect(m_socketTimeout, &QTimer::timeout, server, &LocalServer::timeoutConnections);
                }
            }
        }
        ++m_runningServers;
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

    for (TcpServer *server : m_tcpServers) {
        Q_EMIT server->engineReady(server);
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

bool CWsgiEngine::init()
{
    if (!initApplication()) {
        qCritical() << "Failed to init application, cheaping...";
        return false;
    }

    return true;
}

#include "moc_cwsgiengine.cpp"
