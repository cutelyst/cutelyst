/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "tcpserver.h"

#include "protocol.h"
#include "protocolhttp.h"
#include "server.h"
#include "socket.h"

#include <Cutelyst/Engine>

#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_TCPSERVER, "cwsgi.tcpserver", QtWarningMsg)

using namespace Cutelyst;

TcpServer::TcpServer(const QString &serverAddress, Protocol *protocol, Server *wsgi, QObject *parent)
    : QTcpServer(parent)
    , m_serverAddress(serverAddress)
    , m_wsgi(wsgi)
    , m_protocol(protocol)
{
    m_engine = qobject_cast<CWsgiEngine *>(parent);

    if (m_wsgi->tcpNodelay()) {
        m_socketOptions.push_back({QAbstractSocket::LowDelayOption, 1});
    }
    if (m_wsgi->soKeepalive()) {
        m_socketOptions.push_back({QAbstractSocket::KeepAliveOption, 1});
    }
    if (m_wsgi->socketSndbuf() != -1) {
        m_socketOptions.push_back({QAbstractSocket::SendBufferSizeSocketOption, m_wsgi->socketSndbuf()});
    }
    if (m_wsgi->socketRcvbuf() != -1) {
        m_socketOptions.push_back({QAbstractSocket::ReceiveBufferSizeSocketOption, m_wsgi->socketRcvbuf()});
    }
}

void TcpServer::incomingConnection(qintptr handle)
{
    auto sock           = new TcpSocket(m_engine, this);
    sock->serverAddress = m_serverAddress;
    sock->protoData     = m_protocol->createData(sock);

    connect(sock, &QIODevice::readyRead, [sock] {
        sock->timeout = false;
        sock->proto->parse(sock, sock);
    });
    connect(sock, &TcpSocket::finished, this, [this, sock] {
        sock->deleteLater();
        if (--m_processing == 0) {
            m_engine->stopSocketTimeout();
        }
    });

    if (Q_LIKELY(sock->setSocketDescriptor(handle, QTcpSocket::ConnectedState, QTcpSocket::ReadWrite | QTcpSocket::Unbuffered))) {
        sock->proto = m_protocol;

        sock->remoteAddress = sock->peerAddress();
        sock->remotePort    = sock->peerPort();
        sock->protoData->setupNewConnection(sock);

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }

        if (++m_processing) {
            m_engine->startSocketTimeout();
        }
    } else {
        delete sock;
    }
}

void TcpServer::shutdown()
{
    close();

    if (m_processing == 0) {
        m_engine->serverShutdown();
    } else {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<TcpSocket *>(child);
            if (socket) {
                connect(socket, &TcpSocket::finished, this, [this]() {
                    if (m_processing == 0) {
                        m_engine->serverShutdown();
                    }
                });
                m_engine->handleSocketShutdown(socket);
            }
        }
    }
}

void TcpServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<TcpSocket *>(child);
            if (socket && !socket->processing && socket->state() == QAbstractSocket::ConnectedState) {
                if (socket->timeout) {
                    qCInfo(CWSGI_TCPSERVER) << "timing out connection" << socket->peerAddress().toString() << socket->peerPort();
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
