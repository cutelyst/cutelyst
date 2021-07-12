/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "tcpsslserver.h"

#include "protocol.h"
#include "socket.h"
#include "server.h"

#ifndef QT_NO_SSL

#include <QSslError>

using namespace Cutelyst;

TcpSslServer::TcpSslServer(const QString &serverAddress, Protocol *protocol, Server *wsgi, QObject *parent)
    : TcpServer(serverAddress, protocol, wsgi, parent)
{

}

void TcpSslServer::incomingConnection(qintptr handle)
{
    auto sock = new SslSocket(m_engine, this);
    sock->protoData = m_protocol->createData(sock);
    sock->setSslConfiguration(m_sslConfiguration);

    connect(sock, &QIODevice::readyRead, this, [sock] () {
        sock->timeout = false;
        sock->proto->parse(sock, sock);
    });
    connect(sock, &SslSocket::finished, this, [this, sock] () {
        sock->deleteLater();
        --m_processing;
    });

    if (Q_LIKELY(sock->setSocketDescriptor(handle, QTcpSocket::ConnectedState, QTcpSocket::ReadWrite | QTcpSocket::Unbuffered))) {
        sock->proto = m_protocol;

        sock->serverAddress = m_serverAddress;
        sock->remoteAddress = sock->peerAddress();
        sock->remotePort = sock->peerPort();
        sock->protoData->setupNewConnection(sock);

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }

        if (++m_processing) {
            m_engine->startSocketTimeout();
        }

        sock->startServerEncryption();
        if (m_http2Protocol) {
            connect(sock, &SslSocket::encrypted, this, [this, sock] () {
                if (sock->sslConfiguration().nextNegotiatedProtocol() == "h2") {
                    sock->proto = m_http2Protocol;
                    sock->protoData = sock->proto->createData(sock);
                }
            });
        }
    } else {
        delete sock;
    }
}

void TcpSslServer::shutdown()
{
    pauseAccepting();

    if (m_processing == 0) {
        m_engine->serverShutdown();
    } else {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<TcpSocket*>(child);
            if (socket) {
                socket->protoData->headerConnection = ProtocolData::HeaderConnectionClose;
                connect(socket, &TcpSocket::finished, this, [this] () {
                    if (!m_processing) {
                        m_engine->serverShutdown();
                    }
                }, Qt::QueuedConnection);
                socket->connectionClose();
            }
        }
    }
}

void TcpSslServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<SslSocket*>(child);
            if (socket && !socket->processing && socket->state() == QAbstractSocket::ConnectedState) {
                if (socket->timeout) {
                    socket->connectionClose();
                } else {
                    socket->timeout = true;
                }
            }
        }
    }
}

void TcpSslServer::setSslConfiguration(const QSslConfiguration &conf)
{
    m_sslConfiguration = conf;
    conf.allowedNextProtocols();
}

void TcpSslServer::setHttp2Protocol(Protocol *protocol)
{
    m_http2Protocol = protocol;
}

#include "moc_tcpsslserver.cpp"

#endif // QT_NO_SSL
