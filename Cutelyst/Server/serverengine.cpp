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

Q_LOGGING_CATEGORY(C_SERVER_ENGINE, "cutelyst.server.engine", QtWarningMsg)

using namespace Cutelyst;
using namespace Qt::StringLiterals;

QByteArray dateHeader();

ServerEngine::ServerEngine(Application *localApp,
                           int workerCore,
                           const QVariantMap &opts,
                           Server *server)
    : Engine(localApp, workerCore, opts)
    , m_lastDate{dateHeader()}
    , m_server(server)
{
    m_lastDateTimer.start();

    if (m_server->socketTimeout()) {
        m_socketTimeout = new QTimer(this);
        m_socketTimeout->setObjectName(u"Cutelyst::socketTimeout"_s);
        m_socketTimeout->setInterval(std::chrono::seconds{m_server->socketTimeout()});
    }

    connect(this, &ServerEngine::shutdown, app(), [this] { Q_EMIT app()->shuttingDown(app()); });

    const QStringList staticMap  = m_server->staticMap();
    const QStringList staticMap2 = m_server->staticMap2();
    if (!staticMap.isEmpty() || !staticMap2.isEmpty()) {
        // NOLINTNEXTLINE
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
            TcpServer *cloneServer = balancer->createServer(this);
            if (cloneServer) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(m_socketTimeout,
                            &QTimer::timeout,
                            cloneServer,
                            &TcpServer::timeoutConnections);
                }

                if (cloneServer->protocol()->type() == Protocol::Type::Http11) {
                    cloneServer->setProtocol(getProtoHttp());
                } else if (cloneServer->protocol()->type() == Protocol::Type::Http2) {
                    cloneServer->setProtocol(getProtoHttp2());
                } else if (cloneServer->protocol()->type() == Protocol::Type::FastCGI1) {
                    cloneServer->setProtocol(getProtoFastCgi());
                }

#ifndef QT_NO_SSL
                if (m_server->httpsH2()) {
                    auto sslServer = qobject_cast<TcpSslServer *>(cloneServer);
                    if (sslServer) {
                        sslServer->setHttp2Protocol(getProtoHttp2());
                    }
                }
#endif // QT_NO_SSL
            }
        }

        const auto localServer = qobject_cast<LocalServer *>(server);
        if (localServer) {
            LocalServer *cloneServer = localServer->createServer(this);
            if (cloneServer) {
                ++m_runningServers;
                if (m_socketTimeout) {
                    connect(m_socketTimeout,
                            &QTimer::timeout,
                            cloneServer,
                            &LocalServer::timeoutConnections);
                }

                if (cloneServer->protocol()->type() == Protocol::Type::Http11) {
                    cloneServer->setProtocol(getProtoHttp());
                } else if (cloneServer->protocol()->type() == Protocol::Type::Http2) {
                    cloneServer->setProtocol(getProtoHttp2());
                } else if (cloneServer->protocol()->type() == Protocol::Type::FastCGI1) {
                    cloneServer->setProtocol(getProtoFastCgi());
                }
            }
        }
    }
}

void ServerEngine::postFork(int workerId)
{
    m_workerId = workerId;

#ifdef Q_OS_UNIX
    UnixFork::setSched(m_server, workerId, workerCore());
#endif

    if (Q_LIKELY(postForkApplication())) {
        Q_EMIT started();
    } else {
        std::cerr << "Application failed to post fork, cheaping worker: " << workerId
                  << ", core: " << workerCore() << '\n';
        Q_EMIT shutdown();
    }
}

QByteArray ServerEngine::dateHeader()
{
    QString ret;
    ret = QLatin1String("\r\nDate: ") + QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                                              u"ddd, dd MMM yyyy hh:mm:ss 'GMT"_s);
    return ret.toLatin1();
}

Protocol *ServerEngine::getProtoHttp()
{
    if (!m_protoHttp) {
        if (m_server->upgradeH2c()) {
            m_protoHttp = new ProtocolHttp(m_server, getProtoHttp2());
        } else {
            m_protoHttp = new ProtocolHttp(m_server);
        }
    }
    return m_protoHttp;
}

ProtocolHttp2 *ServerEngine::getProtoHttp2()
{
    if (!m_protoHttp2) {
        m_protoHttp2 = new ProtocolHttp2(m_server);
    }
    return m_protoHttp2;
}

Protocol *ServerEngine::getProtoFastCgi()
{
    if (!m_protoFcgi) {
        m_protoFcgi = new ProtocolFastCGI(m_server);
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
