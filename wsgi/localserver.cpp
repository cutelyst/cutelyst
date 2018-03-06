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
#include "localserver.h"
#include "socket.h"
#include "protocol.h"
#include "wsgi.h"

#include <Cutelyst/Engine>

#include <QSocketNotifier>
#include <QDateTime>

#ifdef Q_OS_UNIX
//#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
//#include <netinet/in.h>
#include <fcntl.h>

static inline int cutelyst_safe_accept(int s, struct sockaddr *addr, uint *addrlen, int flags = 0);
#endif

using namespace CWSGI;


LocalServer::LocalServer(WSGI *wsgi, QObject *parent) : QLocalServer(parent)
  , m_wsgi(wsgi)
{
}

void LocalServer::setProtocol(Protocol *protocol)
{
    m_protocol = protocol;
}

LocalServer *LocalServer::createServer(CWsgiEngine *engine) const
{
    auto server = new LocalServer(m_wsgi, engine);
    server->setProtocol(m_protocol);
    server->m_engine = engine;

#ifdef Q_OS_UNIX
    server->m_socket = socket();
    server->m_socketNotifier = new QSocketNotifier(server->m_socket, QSocketNotifier::Read, server);
    server->m_socketNotifier->setEnabled(false);
    connect(server->m_socketNotifier, &QSocketNotifier::activated, server, &LocalServer::socketNotifierActivated);
#else
    if (server->listen(socket())) {
        server->pauseAccepting();
    } else {
        qFatal("Failed to set server socket descriptor");
    }
#endif

    connect(engine, &CWsgiEngine::started, server, &LocalServer::resumeAccepting);
    connect(engine, &CWsgiEngine::shutdown, server, &LocalServer::shutdown);

    return server;
}

void LocalServer::pauseAccepting()
{
    auto notifier = socketDescriptorNotifier();
    if (notifier) {
        notifier->setEnabled(false);
    }
}

void LocalServer::resumeAccepting()
{
#ifdef Q_OS_UNIX
    m_socketNotifier->setEnabled(true);
#else
    auto notifier = socketDescriptorNotifier();
    if (notifier) {
        notifier->setEnabled(true);
    }
#endif
}

void LocalServer::incomingConnection(quintptr handle)
{
    LocalSocket *sock;
    if (!m_socks.empty()) {
        sock = m_socks.back();
        m_socks.pop_back();
    } else {
        sock = new LocalSocket(m_wsgi, m_engine, this);

        connect(sock, &QIODevice::readyRead, [sock] () {
            sock->protoRequest->timeout = false;
            sock->protoRequest->proto->readyRead(sock, sock);
        });
        connect(sock, &LocalSocket::finished, [this] (LocalSocket *obj) {
            m_socks.push_back(obj);
            if (--m_processing == 0) {
                m_engine->stopSocketTimeout();
            }
        });
    }

    if (Q_LIKELY(sock->setSocketDescriptor(handle))) {
        sock->resetSocket();

        sock->protoRequest->proto = m_protocol;
        sock->protoRequest->serverAddress = QStringLiteral("localhost");
        if (++m_processing) {
            m_engine->startSocketTimeout();
        }
    } else {
        m_socks.push_back(sock);
    }
}

qintptr LocalServer::socket() const
{
    QSocketNotifier *notifier = socketDescriptorNotifier();
    if (notifier) {
        return notifier->socket();
    }

    return 0;
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
                socket->protoRequest->headerConnection = ProtoRequest::HeaderConnectionClose;
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
            if (socket && !socket->protoRequest->processing && socket->state() == QLocalSocket::ConnectedState) {
                if (socket->protoRequest->timeout) {
                    socket->connectionClose();
                } else {
                    socket->protoRequest->timeout = true;
                }
            }
        }
    }
}

Protocol *LocalServer::protocol() const
{
    return m_protocol;
}

QSocketNotifier *LocalServer::socketDescriptorNotifier() const
{
    QSocketNotifier *ret = nullptr;
    // THIS IS A HACK
    // QLocalServer does not expose the socket
    // descriptor, so we get it from it's QSocketNotifier child
    // if this breaks it we fail with an error.
    const auto childrenL = children();
    for (auto child : childrenL) {
        auto notifier = qobject_cast<QSocketNotifier*>(child);
        if (notifier) {
            ret = notifier;
            break;
        }
    }

    return ret;
}

#ifdef Q_OS_UNIX
void LocalServer::socketNotifierActivated()
{
    if (-1 == m_socket)
        return;

    ::sockaddr_un addr;
    uint length = sizeof(sockaddr_un);
    int connectedSocket = cutelyst_safe_accept(m_socket, (sockaddr *)&addr, &length);
    if (-1 != connectedSocket) {
        incomingConnection(connectedSocket);
    }
}

// Tru64 redefines accept -> _accept with _XOPEN_SOURCE_EXTENDED
static inline int cutelyst_safe_accept(int s, struct sockaddr *addr, uint *addrlen, int flags)
{
    Q_ASSERT((flags & ~O_NONBLOCK) == 0);

    int fd;
#ifdef QT_THREADSAFE_CLOEXEC
    // use accept4
    int sockflags = SOCK_CLOEXEC;
    if (flags & O_NONBLOCK)
        sockflags |= SOCK_NONBLOCK;
# if defined(Q_OS_NETBSD)
    fd = ::paccept(s, addr, static_cast<socklen_t *>(addrlen), NULL, sockflags);
# else
    fd = ::accept4(s, addr, static_cast<socklen_t *>(addrlen), sockflags);
# endif
    return fd;
#else
    fd = ::accept(s, addr, static_cast<socklen_t *>(addrlen));
    if (fd == -1)
        return -1;

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    // set non-block too?
    if (flags & O_NONBLOCK)
        ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);

    return fd;
#endif
}
#endif // Q_OS_UNIX

#include "moc_localserver.cpp"
