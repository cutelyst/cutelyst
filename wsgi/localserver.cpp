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
#include "localserver.h"
#include "socket.h"
#include "protocol.h"
#include "wsgi.h"

#include <Cutelyst/Engine>

#include <QSocketNotifier>
#include <QDateTime>

using namespace CWSGI;

LocalServer::LocalServer(const QString &serverAddress, Protocol *protocol, WSGI *wsgi, QObject *parent) : QLocalServer(parent)
  , m_wsgi(wsgi)
  , m_protocol(protocol)
{
    m_serverAddress = serverAddress;
    m_engine = qobject_cast<CWsgiEngine*>(parent);
}

bool LocalServer::setSocketDescriptor(qintptr socketDescriptor)
{
    bool ret = false;
    if (listen(socketDescriptor)) {
        // THIS IS A HACK
        // QLocalServer does not expose the socket
        // descriptor, so we get it from it's QSocketNotifier child
        // if this breaks it we fail with an error.
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto notifier = qobject_cast<QSocketNotifier*>(child);
            if (notifier) {
                m_notifier = notifier;
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void LocalServer::pauseAccepting()
{
    Q_ASSERT(m_notifier);
    m_notifier->setEnabled(false);
}

void LocalServer::resumeAccepting()
{
    Q_ASSERT(m_notifier);
    m_notifier->setEnabled(true);
}

void LocalServer::incomingConnection(quintptr handle)
{
    LocalSocket *sock;
    if (!m_socks.empty()) {
        sock = m_socks.back();
        m_socks.pop_back();
    } else {
        sock = new LocalSocket(m_wsgi, this);
        sock->engine = m_engine;
        sock->proto = m_protocol;

        connect(sock, &QIODevice::readyRead, [sock] () {
            sock->timeout = false;
            sock->proto->readyRead(sock, sock);
        });
        connect(sock, &LocalSocket::finished, [this] (LocalSocket *obj) {
            m_socks.push_back(obj);
            if (--m_processing == 0) {
                m_engine->stopSocketTimeout();
            }
        });
    }

    if (sock->setSocketDescriptor(handle)) {
        sock->resetSocket();
        sock->serverAddress = m_serverAddress;
        if (++m_processing) {
            m_engine->startSocketTimeout();
        }
    } else {
        m_socks.push_back(sock);
    }
}

void LocalServer::shutdown()
{
    pauseAccepting();

    if (m_processing == 0) {
        m_engine->serverShutdown();
    } else {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<LocalSocket*>(child);
            if (socket) {
                socket->headerClose = Socket::HeaderCloseClose;
                connect(socket, &LocalSocket::finished, [this] () {
                    if (!m_processing) {
                        m_engine->serverShutdown();
                    }
                });
            }
        }
    }
}

void LocalServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<LocalSocket*>(child);
            if (socket && !socket->processing && socket->state() == QLocalSocket::ConnectedState) {
                if (socket->timeout) {
                    socket->connectionClose();
                } else {
                    socket->timeout = true;
                }
            }
        }
    }
}

#include "moc_localserver.cpp"
