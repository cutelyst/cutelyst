/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "tcpserverbalancer.h"

#include "wsgi.h"
#include "cwsgiengine.h"
#include "tcpserver.h"
#include "tcpsslserver.h"

#include <QFile>
#include <QMutexLocker>

#include <QSslKey>
#include <QSslKey>

#include <iostream>

static QMutex mutex;

using namespace CWSGI;

TcpServerBalancer::TcpServerBalancer(WSGI *wsgi) : QTcpServer(wsgi)
  , m_wsgi(wsgi)
{
}

TcpServerBalancer::~TcpServerBalancer()
{
    delete m_sslConfiguration;
}

bool TcpServerBalancer::listen(const QString &line, Protocol *protocol, bool secure)
{
    m_protocol = protocol;

    const QString addressString = line
            .section(QLatin1Char(':'), 0, 0)
            .section(QLatin1Char(','), 0, 0);

    QHostAddress address;
    if (addressString.isEmpty()) {
        address = QHostAddress(QHostAddress::Any);
    } else {
        address.setAddress(addressString);
    }

    const QString afterColon = line.section(QLatin1Char(':'), 1);
    const QString portString = afterColon.section(QLatin1Char(','), 0, 0);

    bool ok;
    quint16 port = portString.toUInt(&ok);
    if (!ok || (port < 1 && port > 35554)) {
        port = 80;
    }

    if (secure) {
        const QString certPath = afterColon.section(QLatin1Char(','), 1, 1);
        auto certFile = new QFile(certPath);
        if (!certFile->open(QFile::ReadOnly)) {
            std::cerr << "Failed to open SSL certificate" << certPath.toLocal8Bit().constData()
                      << certFile->errorString().toLocal8Bit().constData() << std::endl;
            exit(1);
        }
        QSslCertificate cert(certFile);
        if (cert.isNull()) {
            std::cerr << "Failed to parse SSL certificate" << std::endl;
            exit(1);
        }

        const QString keyPath = afterColon.section(QLatin1Char(','), 2, 2);
        auto keyFile = new QFile(keyPath);
        if (!keyFile->open(QFile::ReadOnly)) {
            std::cerr << "Failed to open SSL private key" << keyPath.toLocal8Bit().constData()
                      << keyFile->errorString().toLocal8Bit().constData() << std::endl;
            exit(1);
        }
        QSslKey key(keyFile, QSsl::Rsa);
        if (key.isNull()) {
            std::cerr << "Failed to parse SSL private key" << std::endl;
            exit(1);
        }

        m_sslConfiguration = new QSslConfiguration;
        m_sslConfiguration->setLocalCertificate(cert);
        m_sslConfiguration->setPrivateKey(key);
    }

    bool ret = QTcpServer::listen(address, port);
    if (ret) {
        pauseAccepting();

        m_serverName = serverAddress().toString() + QLatin1Char(':') + QString::number(port);
    } else {
        std::cerr << "Failed to listen on TCP: " << line.toUtf8().constData()
                  << " : " << errorString().toUtf8().constData() << std::endl;
        exit(1);
    }
    return ret;
}

void TcpServerBalancer::setBalancer(bool enable)
{
    m_balancer = enable;
}

void TcpServerBalancer::incomingConnection(qintptr handle)
{
    TcpServer *serverIdle = m_servers.at(m_currentServer++ % m_servers.size());

    serverIdle->createConnection(handle);
}

TcpServer *TcpServerBalancer::addServer(CWsgiEngine *engine)
{
    QMutexLocker locker(&mutex);

    TcpServer *server;
    if (m_sslConfiguration) {
        auto sslServer = new TcpSslServer(m_serverName, m_protocol, m_wsgi, engine);
        sslServer->setSslConfiguration(*m_sslConfiguration);
        server = sslServer;
    } else {
        server = new TcpServer(m_serverName, m_protocol, m_wsgi, engine);
    }

    if (!m_balancer) {
        server->pauseAccepting();

        if (server->setSocketDescriptor(socketDescriptor())) {
            connect(engine, &CWsgiEngine::started, server, &TcpServer::resumeAccepting);
        } else {
            qFatal("Failed to set server socket descriptor");
        }
    } else {
        connect(engine, &CWsgiEngine::started, this, &TcpServerBalancer::resumeAccepting, Qt::QueuedConnection);
        connect(server, &TcpServer::createConnection, server, &TcpServer::incomingConnection, Qt::QueuedConnection);
    }
    connect(engine, &CWsgiEngine::shutdown, server, &TcpServer::shutdown);

    m_servers.push_back(server);

    return server;
}
