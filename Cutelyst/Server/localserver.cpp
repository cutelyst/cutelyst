/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "localserver.h"

#include "protocol.h"
#include "server.h"
#include "socket.h"

#include <Cutelyst/Engine>

#include <QDateTime>
#include <QSocketNotifier>

#ifdef Q_OS_UNIX
#    include <fcntl.h>
#    include <sys/socket.h>
#    include <sys/un.h>

namespace {
// Tru64 redefines accept -> _accept with _XOPEN_SOURCE_EXTENDED
inline int cutelyst_safe_accept(int s, struct sockaddr *addr, uint *addrlen, int flags = 0)
{
    Q_ASSERT((flags & ~O_NONBLOCK) == 0);

    int fd;
#    ifdef QT_THREADSAFE_CLOEXEC
    // use accept4
    int sockflags = SOCK_CLOEXEC;
    if (flags & O_NONBLOCK) {
        sockflags |= SOCK_NONBLOCK;
    }
#        if defined(Q_OS_NETBSD)
    fd = ::paccept(s, addr, static_cast<socklen_t *>(addrlen), NULL, sockflags);
#        else
    fd = ::accept4(s, addr, static_cast<socklen_t *>(addrlen), sockflags);
#        endif
    return fd;
#    else
    fd = ::accept(s, addr, static_cast<socklen_t *>(addrlen));
    if (fd == -1) {
        return -1;
    }

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    // set non-block too?
    if (flags & O_NONBLOCK) {
        ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);
    }

    return fd;
#    endif
}
} // namespace
#endif

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

LocalServer::LocalServer(Server *server, QObject *parent)
    : QLocalServer(parent)
    , m_server(server)
{
}

void LocalServer::setProtocol(Protocol *protocol)
{
    m_protocol = protocol;
}

LocalServer *LocalServer::createServer(ServerEngine *engine) const
{
    auto server = new LocalServer(m_server, engine);
    server->setProtocol(m_protocol);
    server->m_engine = engine;

#ifdef Q_OS_UNIX
    server->m_socket         = socket();
    server->m_socketNotifier = new QSocketNotifier(server->m_socket, QSocketNotifier::Read, server);
    server->m_socketNotifier->setEnabled(false);
    connect(server->m_socketNotifier,
            &QSocketNotifier::activated,
            server,
            &LocalServer::socketNotifierActivated);
#else
    if (server->listen(socket())) {
        server->pauseAccepting();
    } else {
        qFatal("Failed to set server socket descriptor");
    }
#endif

    connect(engine, &ServerEngine::started, server, &LocalServer::resumeAccepting);
    connect(engine, &ServerEngine::shutdown, server, &LocalServer::shutdown);

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
    auto sock       = new LocalSocket(m_engine, this);
    sock->protoData = m_protocol->createData(sock);

    connect(sock, &QIODevice::readyRead, [sock]() {
        sock->timeout = false;
        sock->proto->parse(sock, sock);
    });
    connect(sock, &LocalSocket::finished, this, [this, sock]() {
        sock->deleteLater();
        if (--m_processing == 0) {
            m_engine->stopSocketTimeout();
        }
    });

    if (Q_LIKELY(sock->setSocketDescriptor(qintptr(handle)))) {
        sock->proto = m_protocol;

        sock->serverAddress = "localhost"_ba;
        if (++m_processing) {
            m_engine->startSocketTimeout();
        }
    } else {
        delete sock;
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
    close();

    if (m_processing == 0) {
        m_engine->serverShutdown();
    } else {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto sock = qobject_cast<LocalSocket *>(child);
            if (sock) {
                connect(sock, &LocalSocket::finished, this, [this]() {
                    if (!m_processing) {
                        m_engine->serverShutdown();
                    }
                });
                m_engine->handleSocketShutdown(sock);
            }
        }
    }
}

void LocalServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto sock = qobject_cast<LocalSocket *>(child);
            if (sock && !sock->processing && sock->state() == QLocalSocket::ConnectedState) {
                if (sock->timeout) {
                    sock->connectionClose();
                } else {
                    sock->timeout = true;
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
        auto notifier = qobject_cast<QSocketNotifier *>(child);
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
    if (-1 == m_socket) {
        return;
    }

    ::sockaddr_un addr;
    uint length = sizeof(sockaddr_un);
    int connectedSocket =
        cutelyst_safe_accept(int(m_socket), reinterpret_cast<sockaddr *>(&addr), &length);
    if (-1 != connectedSocket) {
        incomingConnection(quintptr(connectedSocket));
    }
}
#endif // Q_OS_UNIX

#include "moc_localserver.cpp"
