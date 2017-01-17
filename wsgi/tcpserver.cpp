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
#include "tcpserver.h"
#include "socket.h"
#include "protocolhttp.h"
#include "protocolfastcgi.h"
#include "wsgi.h"

#include <Cutelyst/Engine>
#include <QDateTime>

using namespace CWSGI;

TcpServer::TcpServer(const QString &serverAddress, int protocol, WSGI *wsgi, QObject *parent) : QTcpServer(parent)
  , m_wsgi(wsgi)
  , m_protocol(protocol)
{
    m_serverAddress = serverAddress;
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
        sock->resetSocket();
    } else {
        sock = new TcpSocket(m_wsgi, this);
        sock->engine = m_engine;
        if (m_protocol == 1) {
            sock->proto = new ProtocolHttp(sock, m_wsgi, sock);
        } else {
            sock->proto = new ProtocolFastCGI(sock, m_wsgi, sock);
        }

        connect(sock, &QIODevice::readyRead, [sock] () {
            sock->proto->readyRead(sock, sock);
        });
        connect(sock, &TcpSocket::finished, [this] (TcpSocket *obj) {
            m_socks.push_back(obj);
        });
    }

    if (sock->setSocketDescriptor(handle)) {
        sock->serverAddress = m_serverAddress;
        sock->remoteAddress = sock->peerAddress();
        sock->remotePort = sock->peerPort();

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }
    } else {
        m_socks.push_back(sock);
    }
}

#include "moc_tcpserver.cpp"
