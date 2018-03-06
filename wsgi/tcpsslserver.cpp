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
#include "wsgi.h"

#include <QSslError>

using namespace CWSGI;

TcpSslServer::TcpSslServer(const QString &serverAddress, CWSGI::Protocol *protocol, CWSGI::WSGI *wsgi, QObject *parent)
    : TcpServer(serverAddress, protocol, wsgi, parent)
{
}

void TcpSslServer::incomingConnection(qintptr handle)
{
    auto sock = new SslSocket(m_wsgi, m_engine, this);
    sock->setSslConfiguration(m_sslConfiguration);

    connect(sock, &QIODevice::readyRead, [sock] () {
        sock->protoRequest->timeout = false;
        sock->protoRequest->proto->readyRead(sock, sock);
    });
    connect(sock, &SslSocket::finished, [this] () {
        --m_processing;
    });
    connect(sock, &SslSocket::disconnected, sock, &SslSocket::deleteLater);

    if (Q_LIKELY(sock->setSocketDescriptor(handle))) {
        sock->resetSocket();

        sock->protoRequest->proto = m_protocol;
        sock->protoRequest->serverAddress = m_serverAddress;
        sock->protoRequest->remoteAddress = sock->peerAddress();
        sock->protoRequest->remotePort = sock->peerPort();

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }

        if (++m_processing) {
            m_engine->startSocketTimeout();
        }

        sock->startServerEncryption();
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
                socket->protoRequest->headerConnection = ProtoRequest::HeaderConnectionClose;
                connect(socket, &TcpSocket::finished, [this] () {
                    if (!m_processing) {
                        m_engine->serverShutdown();
                    }
                });
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
            if (socket && !socket->protoRequest->processing && socket->state() == QAbstractSocket::ConnectedState) {
                if (socket->protoRequest->timeout) {
                    socket->connectionClose();
                } else {
                    socket->protoRequest->timeout = true;
                }
            }
        }
    }
}

void TcpSslServer::setSslConfiguration(const QSslConfiguration &conf)
{
    m_sslConfiguration = conf;
}

#include "moc_tcpsslserver.cpp"
