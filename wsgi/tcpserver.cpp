/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include <Cutelyst/Engine>
#include <QDateTime>
#include <QSocketNotifier>

using namespace CWSGI;

TcpServer::TcpServer(const QString &serverAddress, WSGI *wsgi, QObject *parent) : QTcpServer(parent)
  , m_wsgi(wsgi)
{
    m_serverAddress = serverAddress;
    m_engine = qobject_cast<CWsgiEngine*>(parent);
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
        sock->serverAddress = m_serverAddress;
        auto proto = new ProtocolHttp(sock, m_wsgi, sock);
        connect(sock, &QIODevice::readyRead, proto, &Protocol::readyRead);
        connect(sock, &TcpSocket::finished, this, &TcpServer::enqueue);
    }

    sock->setSocketDescriptor(handle);
    sock->start = QDateTime::currentMSecsSinceEpoch();
}

void TcpServer::enqueue()
{
    m_socks.push_back(qobject_cast<TcpSocket*>(sender()));
}

#include "moc_tcpserver.cpp"
