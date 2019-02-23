/*
 * Copyright (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "tcpserverbalancer.h"

#include "wsgi.h"
#include "cwsgiengine.h"
#include "tcpserver.h"
#include "tcpsslserver.h"

#include <QFile>
#include <QLoggingCategory>

#include <QSslKey>

#include <iostream>

#ifdef Q_OS_LINUX
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif


Q_LOGGING_CATEGORY(CWSGI_BALANCER, "wsgi.tcp_server_balancer", QtWarningMsg)

using namespace CWSGI;

#ifdef Q_OS_LINUX
int listenReuse(const QHostAddress &address, quint16 port, bool startListening);
#endif

TcpServerBalancer::TcpServerBalancer(WSGI *wsgi) : QTcpServer(wsgi)
  , m_wsgi(wsgi)
{
}

TcpServerBalancer::~TcpServerBalancer()
{
#ifndef QT_NO_SSL
    delete m_sslConfiguration;
#endif // QT_NO_SSL
}

bool TcpServerBalancer::listen(const QString &line, Protocol *protocol, bool secure)
{
    m_protocol = protocol;

    int commaPos = line.indexOf(QLatin1Char(','));
    const QString addressPortString = line.mid(0, commaPos);

    QString addressString;
    int closeBracketPos = addressPortString.indexOf(QLatin1Char(']'));
    if (closeBracketPos != -1) {
        if (!line.startsWith(QLatin1Char('['))) {
            std::cerr << "Failed to parse address: " << qPrintable(addressPortString) << std::endl;
            exit(1);
        }
        addressString = addressPortString.mid(1, closeBracketPos - 1);
    } else {
        addressString = addressPortString.section(QLatin1Char(':'), 0, -2);
    }
    const QString portString = addressPortString.section(QLatin1Char(':'), -1);

    QHostAddress address;
    if (addressString.isEmpty()) {
        address = QHostAddress(QHostAddress::Any);
    } else {
        address.setAddress(addressString);
    }

    bool ok;
    quint16 port = portString.toUInt(&ok);
    if (!ok || (port < 1 || port > 35554)) {
        port = 80;
    }

#ifndef QT_NO_SSL
    if (secure) {
        if (commaPos == -1) {
            std::cerr << "No SSL certificate specified" << std::endl;
            exit(1);
        }

        const QString sslString = line.mid(commaPos + 1);
        const QString certPath = sslString.section(QLatin1Char(','), 0, 0);
        QFile certFile(certPath);
        if (!certFile.open(QFile::ReadOnly)) {
            std::cerr << "Failed to open SSL certificate" << qPrintable(certPath)
                      << qPrintable(certFile.errorString()) << std::endl;
            exit(1);
        }
        QSslCertificate cert(&certFile);
        if (cert.isNull()) {
            std::cerr << "Failed to parse SSL certificate" << std::endl;
            exit(1);
        }

        const QString keyPath = sslString.section(QLatin1Char(','), 1, 1);
        QFile keyFile(keyPath);
        if (!keyFile.open(QFile::ReadOnly)) {
            std::cerr << "Failed to open SSL private key" << qPrintable(keyPath)
                      << qPrintable(keyFile.errorString()) << std::endl;
            exit(1);
        }
        QSslKey key(&keyFile, QSsl::Rsa);
        if (key.isNull()) {
            std::cerr << "Failed to parse SSL private key" << std::endl;
            exit(1);
        }

        m_sslConfiguration = new QSslConfiguration;
        m_sslConfiguration->setLocalCertificate(cert);
        m_sslConfiguration->setPrivateKey(key);
        m_sslConfiguration->setPeerVerifyMode(QSslSocket::VerifyNone); // prevent asking for client certificate
        if (m_wsgi->httpsH2()) {
            m_sslConfiguration->setAllowedNextProtocols({ QByteArrayLiteral("h2") });
        }
    }
#endif // QT_NO_SSL

    m_address = address;
    m_port = port;

#ifdef Q_OS_LINUX
    if (m_wsgi->reusePort()) {
        int socket = listenReuse(address, port, false);
        if (socket > 0) {
            setSocketDescriptor(socket);
            pauseAccepting();
        } else {
            std::cerr << "Failed to listen on TCP: " << qPrintable(line)
                      << " : " << qPrintable(errorString()) << std::endl;
            exit(1);
        }
    } else {
#endif
        bool ret = QTcpServer::listen(address, port);
        if (ret) {
            pauseAccepting();
        } else {
            std::cerr << "Failed to listen on TCP: " << qPrintable(line)
                      << " : " << qPrintable(errorString()) << std::endl;
            exit(1);
        }
#ifdef Q_OS_LINUX
    }
#endif

    m_serverName = serverAddress().toString() + QLatin1Char(':') + QString::number(port);
    return true;
}

#ifdef Q_OS_LINUX
// UnixWare 7 redefines socket -> _socket
static inline int qt_safe_socket(int domain, int type, int protocol, int flags = 0)
{
    Q_ASSERT((flags & ~O_NONBLOCK) == 0);

    int fd;
#ifdef QT_THREADSAFE_CLOEXEC
    int newtype = type | SOCK_CLOEXEC;
    if (flags & O_NONBLOCK)
        newtype |= SOCK_NONBLOCK;
    fd = ::socket(domain, newtype, protocol);
    return fd;
#else
    fd = ::socket(domain, type, protocol);
    if (fd == -1)
        return -1;

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    // set non-block too?
    if (flags & O_NONBLOCK)
        ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);

    return fd;
#endif
}

int createNewSocket(QAbstractSocket::NetworkLayerProtocol &socketProtocol)
{
    int protocol = 0;

    int domain = (socketProtocol == QAbstractSocket::IPv6Protocol
                  || socketProtocol == QAbstractSocket::AnyIPProtocol) ? AF_INET6 : AF_INET;
    int type = SOCK_STREAM;

    int socket = qt_safe_socket(domain, type, protocol, O_NONBLOCK);
    if (socket < 0 && socketProtocol == QAbstractSocket::AnyIPProtocol && errno == EAFNOSUPPORT) {
        domain = AF_INET;
        socket = qt_safe_socket(domain, type, protocol, O_NONBLOCK);
        socketProtocol = QAbstractSocket::IPv4Protocol;
    }

    if (socket < 0) {
        int ecopy = errno;
        switch (ecopy) {
        case EPROTONOSUPPORT:
        case EAFNOSUPPORT:
        case EINVAL:
            qCDebug(CWSGI_BALANCER) << "setError(QAbstractSocket::UnsupportedSocketOperationError, ProtocolUnsupportedErrorString)";
            break;
        case ENFILE:
        case EMFILE:
        case ENOBUFS:
        case ENOMEM:
            qCDebug(CWSGI_BALANCER) << "setError(QAbstractSocket::SocketResourceError, ResourceErrorString)";
            break;
        case EACCES:
            qCDebug(CWSGI_BALANCER) << "setError(QAbstractSocket::SocketAccessError, AccessErrorString)";
            break;
        default:
            break;
        }

#if defined (QNATIVESOCKETENGINE_DEBUG)
        qCDebug(CWSGI_BALANCER, "QNativeSocketEnginePrivate::createNewSocket(%d, %d) == false (%s)",
                socketType, socketProtocol,
                strerror(ecopy));
#endif

        return false;
    }

#if defined (QNATIVESOCKETENGINE_DEBUG)
    qCDebug(CWSGI_BALANCER, "QNativeSocketEnginePrivate::createNewSocket(%d, %d) == true",
            socketType, socketProtocol);
#endif

    return socket;
}

union qt_sockaddr {
    sockaddr a;
    sockaddr_in a4;
    sockaddr_in6 a6;
};

#  define QT_SOCKLEN_T int
#define QT_SOCKET_BIND          ::bind

namespace {
namespace SetSALen {
    template <typename T> void set(T *sa, typename std::enable_if<(&T::sa_len, true), QT_SOCKLEN_T>::type len)
    { sa->sa_len = len; }
    template <typename T> void set(T *sin6, typename std::enable_if<(&T::sin6_len, true), QT_SOCKLEN_T>::type len)
    { sin6->sin6_len = len; }
    template <typename T> void set(T *, ...) {}
}
}

void setPortAndAddress(quint16 port, const QHostAddress &address, QAbstractSocket::NetworkLayerProtocol socketProtocol, qt_sockaddr *aa, int *sockAddrSize)
{
    if (address.protocol() == QAbstractSocket::IPv6Protocol
        || address.protocol() == QAbstractSocket::AnyIPProtocol
        || socketProtocol == QAbstractSocket::IPv6Protocol
        || socketProtocol == QAbstractSocket::AnyIPProtocol) {
        memset(&aa->a6, 0, sizeof(sockaddr_in6));
        aa->a6.sin6_family = AF_INET6;
//#if QT_CONFIG(networkinterface)
//            aa->a6.sin6_scope_id = scopeIdFromString(address.scopeId());
//#endif
        aa->a6.sin6_port = htons(port);
        Q_IPV6ADDR tmp = address.toIPv6Address();
        memcpy(&aa->a6.sin6_addr, &tmp, sizeof(tmp));
        *sockAddrSize = sizeof(sockaddr_in6);
        SetSALen::set(&aa->a, sizeof(sockaddr_in6));
    } else {
        memset(&aa->a, 0, sizeof(sockaddr_in));
        aa->a4.sin_family = AF_INET;
        aa->a4.sin_port = htons(port);
        aa->a4.sin_addr.s_addr = htonl(address.toIPv4Address());
        *sockAddrSize = sizeof(sockaddr_in);
        SetSALen::set(&aa->a, sizeof(sockaddr_in));
    }
}

bool nativeBind(int socketDescriptor, const QHostAddress &address, quint16 port)
{
    qt_sockaddr aa;
    int sockAddrSize;
    setPortAndAddress(port, address, address.protocol(), &aa, &sockAddrSize);

#ifdef IPV6_V6ONLY
    if (aa.a.sa_family == AF_INET6) {
        int ipv6only = 0;
        if (address.protocol() == QAbstractSocket::IPv6Protocol)
            ipv6only = 1;
        //default value of this socket option varies depending on unix variant (or system configuration on BSD), so always set it explicitly
        ::setsockopt(socketDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only) );
    }
#endif

    int bindResult = ::bind(socketDescriptor, &aa.a, sockAddrSize);
    if (bindResult < 0 && errno == EAFNOSUPPORT && address.protocol() == QAbstractSocket::AnyIPProtocol) {
        // retry with v4
        aa.a4.sin_family = AF_INET;
        aa.a4.sin_port = htons(port);
        aa.a4.sin_addr.s_addr = htonl(address.toIPv4Address());
        sockAddrSize = sizeof(aa.a4);
        bindResult = QT_SOCKET_BIND(socketDescriptor, &aa.a, sockAddrSize);
    }

    if (bindResult < 0) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
        int ecopy = errno;
#endif
//        switch(errno) {
//        case EADDRINUSE:
//            setError(QAbstractSocket::AddressInUseError, AddressInuseErrorString);
//            break;
//        case EACCES:
//            setError(QAbstractSocket::SocketAccessError, AddressProtectedErrorString);
//            break;
//        case EINVAL:
//            setError(QAbstractSocket::UnsupportedSocketOperationError, OperationUnsupportedErrorString);
//            break;
//        case EADDRNOTAVAIL:
//            setError(QAbstractSocket::SocketAddressNotAvailableError, AddressNotAvailableErrorString);
//            break;
//        default:
//            break;
//        }

#if defined (QNATIVESOCKETENGINE_DEBUG)
        qCDebug(CWSGI_BALANCER, "QNativeSocketEnginePrivate::nativeBind(%s, %i) == false (%s)",
                address.toString().toLatin1().constData(), port, strerror(ecopy));
#endif

        return false;
    }

#if defined (QNATIVESOCKETENGINE_DEBUG)
    qCDebug(CWSGI_BALANCER, "QNativeSocketEnginePrivate::nativeBind(%s, %i) == true",
            address.toString().toLatin1().constData(), port);
#endif
//    socketState = QAbstractSocket::BoundState;
    return true;
}

int listenReuse(const QHostAddress &address, quint16 port, bool startListening)
{
    QAbstractSocket::NetworkLayerProtocol proto = address.protocol();

    int socket = createNewSocket(proto);
    if (socket < 0) {
        qCCritical(CWSGI_BALANCER) << "Failed to create new socket";
        return -1;
    }

    int optval = 1;
    if (::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
        qCCritical(CWSGI_BALANCER) << "Failed to set SO_REUSEPORT on socket" << socket;
        return -1;
    }

    if (!nativeBind(socket, address, port)) {
        qCCritical(CWSGI_BALANCER) << "Failed to bind to socket" << socket;
        return -1;
    }

    if (startListening && ::listen(socket, 100) < 0) {
        qCCritical(CWSGI_BALANCER) << "Failed to listen to socket" << socket;
        return -1;
    }

    return socket;
}
#endif // Q_OS_LINUX

void TcpServerBalancer::setBalancer(bool enable)
{
    m_balancer = enable;
}

void TcpServerBalancer::incomingConnection(qintptr handle)
{
    TcpServer *serverIdle = m_servers.at(m_currentServer++ % m_servers.size());

    Q_EMIT serverIdle->createConnection(handle);
}

TcpServer *TcpServerBalancer::createServer(CWsgiEngine *engine)
{
    TcpServer *server;
    if (m_sslConfiguration) {
#ifndef QT_NO_SSL
        auto sslServer = new TcpSslServer(m_serverName, m_protocol, m_wsgi, engine);
        sslServer->setSslConfiguration(*m_sslConfiguration);
        server = sslServer;
#endif //QT_NO_SSL
    } else {
        server = new TcpServer(m_serverName, m_protocol, m_wsgi, engine);
    }
    connect(engine, &CWsgiEngine::shutdown, server, &TcpServer::shutdown);

    if (m_balancer) {
        connect(engine, &CWsgiEngine::started, this, [=] () {
            m_servers.push_back(server);
            resumeAccepting();
        }, Qt::QueuedConnection);
        connect(server, &TcpServer::createConnection, server, &TcpServer::incomingConnection, Qt::QueuedConnection);
    } else {

#ifdef Q_OS_LINUX
        if (m_wsgi->reusePort()) {
            connect(engine, &CWsgiEngine::started, this, [=] () {
                int socket = listenReuse(m_address, m_port, true);
                if (!server->setSocketDescriptor(socket)) {
                    qFatal("Failed to set server socket descriptor, reuse-port");
                }
            }, Qt::DirectConnection);
            return server;
        }
#endif

        if (server->setSocketDescriptor(socketDescriptor())) {
            server->pauseAccepting();
            connect(engine, &CWsgiEngine::started, server, &TcpServer::resumeAccepting, Qt::DirectConnection);
        } else {
            qFatal("Failed to set server socket descriptor");
        }
    }

    return server;
}

#include "moc_tcpserverbalancer.cpp"
