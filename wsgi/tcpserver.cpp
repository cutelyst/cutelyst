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
#include "tcpserver.h"
#include "socket.h"
#include "protocol.h"
#include "wsgi.h"

#include <Cutelyst/Engine>
#include <QDateTime>

using namespace CWSGI;

TcpServer::TcpServer(const QString &serverAddress, Protocol *protocol, WSGI *wsgi, QObject *parent) : QTcpServer(parent)
  , m_serverAddress(serverAddress)
  , m_wsgi(wsgi)
  , m_protocol(protocol)
{
    m_engine = qobject_cast<CWsgiEngine*>(parent);

    if (m_wsgi->tcpNodelay()) {
        m_socketOptions.push_back({ QAbstractSocket::LowDelayOption, 1 });
    }
    if (m_wsgi->soKeepalive()) {
        m_socketOptions.push_back({ QAbstractSocket::KeepAliveOption, 1 });
    }
    if (m_wsgi->socketSndbuf() != -1) {
        m_socketOptions.push_back({ QAbstractSocket::SendBufferSizeSocketOption, m_wsgi->socketSndbuf() });
    }
    if (m_wsgi->socketRcvbuf() != -1) {
        m_socketOptions.push_back({ QAbstractSocket::ReceiveBufferSizeSocketOption, m_wsgi->socketRcvbuf() });
    }
}

void TcpServer::incomingConnection(qintptr handle)
{
    TcpSocket *sock;
    if (!m_socks.empty()) {
        sock = m_socks.back();
        m_socks.pop_back();
    } else {
        sock = new TcpSocket(m_engine, this);
        sock->serverAddress = m_serverAddress;
        sock->protoData = m_protocol->createData(sock);

        connect(sock, &QIODevice::readyRead, [sock] () {
            sock->timeout = false;
            sock->proto->parse(sock, sock);
        });
        connect(sock, &TcpSocket::finished, this, [this, sock] () {
            sock->resetSocket();
            m_socks.push_back(sock);
            --m_processing;
        }, Qt::QueuedConnection);
    }

    if (Q_LIKELY(sock->setSocketDescriptor(handle))) {
        sock->proto = m_protocol;

        sock->remoteAddress = sock->peerAddress();
        sock->remotePort = sock->peerPort();
        sock->protoData->setupNewConnection(sock);

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }

        if (++m_processing) {
            m_engine->startSocketTimeout();
        }
    } else {
        m_socks.push_back(sock);
    }
}

void TcpServer::shutdown()
{
    if (isListening()) {
        pauseAccepting();
    }

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
                });
            }
        }
    }
}

void TcpServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<TcpSocket*>(child);
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

Protocol *TcpServer::protocol() const
{
    return m_protocol;
}

void TcpServer::setProtocol(Protocol *protocol)
{
    m_protocol = protocol;
}

#include "moc_tcpserver.cpp"
