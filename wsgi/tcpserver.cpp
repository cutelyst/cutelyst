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

TcpServer::TcpServer(Protocol *proto, QObject *parent) : QTcpServer(parent)
  , m_proto(proto)
{
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
        sock = new TcpSocket(this);
        sock->engine = m_engine;
        static QString serverAddr = serverAddress().toString();
        sock->serverAddress = serverAddr;
        connect(sock, &QIODevice::readyRead, m_proto, &Protocol::readyRead);
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
