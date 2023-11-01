/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "serverengine.h"

#include "config.h"
#include "localserver.h"
#include "protocol.h"
#include "protocolfastcgi.h"
#include "protocolhttp.h"
#include "protocolhttp2.h"
#include "protocolwebsocket.h"
#include "server.h"
#include "socket.h"
#include "staticmap.h"
#include "tcpserver.h"
#include "tcpserverbalancer.h"
#include "tcpsslserver.h"

#ifdef Q_OS_UNIX
#    include "unixfork.h"
#endif

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Request>
#include <Cutelyst/Response>
#include <iostream>
#include <typeinfo>

#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_ENGINE, "cutelyst.server.engine", QtWarningMsg)

using namespace Cutelyst;

QByteArray dateHeader();

ServerEngine::ServerEngine(Application *localApp,
                           int workerCore,
                           const QVariantMap &opts,
                           Server *wsgi)
    : Engine(localApp, workerCore, opts)
    , m_wsgi(wsgi)
{
    m_lastDate = dateHeader();
    m_lastDateTimer.start();

    if (m_wsgi->socketTimeout()) {
        m_socketTimeout = new QTimer(this);
        m_socketTimeout->setObjectName(QStringLiteral("Cutelyst::socketTimeout"));
        m_socketTimeout->setInterval(std::chrono::seconds{m_wsgi->socketTimeout()});
    }

    connect(this, &ServerEngine::shutdown, app(), [this] { Q_EMIT app()->shuttingDown(app()); });

    const QStringList staticMap  = m_wsgi->staticMap();
    const QStringList staticMap2 = m_wsgi->staticMap2();
    if (!staticMap.isEmpty() || !staticMap2.isEmpty()) {
        auto staticMapPlugin = new StaticMap(app());

        for (const QString &part : staticMap) {
            staticMapPlugin->addStaticMap(
                part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), false);
        }

        for (const QString &part : staticMap2) {
            staticMapPlugin->addStaticMap(
                part.section(QLatin1Char('='), 0, 0), part.section(QLatin1Char('='), 1, 1), true);
        }
    }
}

ServerEngine::~ServerEngine()
{
    delete m_protoFcgi;
    delete m_protoHttp;
    delete m_protoHttp2;
}

int ServerEngine::workerId() const
{
    return m_workerId;
}

void ServerEngine::setServers(const std::vector<QObject *> &servers)
{
    for (QObject *server : servers) {
        auto balancer = qobject_cast<TcpServerBalancer *>(server);
        if (balancer) {
            TcpServer *server = balancer->createServer(this);
            if (server) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(
                        m_socketTimeout, &QTimer::timeout, server, &TcpServer::timeoutConnections);
                }

                if (server->protocol()->type() == Protocol::Type::Http11) {
                    server->setProtocol(getProtoHttp());
                } else if (server->protocol()->type() == Protocol::Type::Http2) {
                    server->setProtocol(getProtoHttp2());
                } else if (server->protocol()->type() == Protocol::Type::FastCGI1) {
                    server->setProtocol(getProtoFastCgi());
                }

#ifndef QT_NO_SSL
                if (m_wsgi->httpsH2()) {
                    auto sslServer = qobject_cast<TcpSslServer *>(server);
                    if (sslServer) {
                        sslServer->setHttp2Protocol(getProtoHttp2());
                    }
                }
#endif // QT_NO_SSL
            }
        }

        auto localServer = qobject_cast<LocalServer *>(server);
        if (localServer) {
            LocalServer *server = localServer->createServer(this);
            if (server) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(m_socketTimeout,
                            &QTimer::timeout,
                            server,
                            &LocalServer::timeoutConnections);
                }

                if (server->protocol()->type() == Protocol::Type::Http11) {
                    server->setProtocol(getProtoHttp());
                } else if (server->protocol()->type() == Protocol::Type::Http2) {
                    server->setProtocol(getProtoHttp2());
                } else if (server->protocol()->type() == Protocol::Type::FastCGI1) {
                    server->setProtocol(getProtoFastCgi());
                }
            }
        }
    }
}

void ServerEngine::postFork(int workerId)
{
    m_workerId = workerId;

#ifdef Q_OS_UNIX
    UnixFork::setSched(m_wsgi, workerId, workerCore());
#endif

    if (Q_LIKELY(postForkApplication())) {
        Q_EMIT started();
    } else {
        std::cerr << "Application failed to post fork, cheaping worker: " << workerId
                  << ", core: " << workerCore() << std::endl;
        Q_EMIT shutdown();
    }
}

QByteArray ServerEngine::dateHeader()
{
    QString ret;
    ret = QLatin1String("\r\nDate: ") +
          QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
    return ret.toLatin1();
}

Protocol *ServerEngine::getProtoHttp()
{
    if (!m_protoHttp) {
        if (m_wsgi->upgradeH2c()) {
            m_protoHttp = new ProtocolHttp(m_wsgi, getProtoHttp2());
        } else {
            m_protoHttp = new ProtocolHttp(m_wsgi);
        }
    }
    return m_protoHttp;
}

ProtocolHttp2 *ServerEngine::getProtoHttp2()
{
    if (!m_protoHttp2) {
        m_protoHttp2 = new ProtocolHttp2(m_wsgi);
    }
    return m_protoHttp2;
}

Protocol *ServerEngine::getProtoFastCgi()
{
    if (!m_protoFcgi) {
        m_protoFcgi = new ProtocolFastCGI(m_wsgi);
    }
    return m_protoFcgi;
}

bool ServerEngine::init()
{
    if (Q_LIKELY(initApplication())) {
        return true;
    }

    return false;
}

void ServerEngine::handleSocketShutdown(Socket *socket)
{
    if (socket->processing == 0) {
        socket->connectionClose();
    } else if (socket->proto->type() == Protocol::Type::Http11Websocket) {
        auto req = static_cast<ProtoRequestHttp *>(socket->protoData);
        req->webSocketClose(Response::CloseCode::CloseCodeGoingAway, {});
    } else {
        socket->protoData->headerConnection = ProtocolData::HeaderConnection::Close;
    }
}

#include "moc_serverengine.cpp"
