/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "tcpsslserver.h"

#include "protocol.h"
#include "server.h"
#include "socket.h"

#ifndef QT_NO_SSL

#    include <QSslError>

using namespace Cutelyst;

TcpSslServer::TcpSslServer(const QByteArray &serverAddress,
                           Protocol *protocol,
                           Server *wsgi,
                           QObject *parent)
    : TcpServer(serverAddress, protocol, wsgi, parent)
{
}

void TcpSslServer::incomingConnection(qintptr handle)
{
    auto sock       = new SslSocket(m_engine, this);
    sock->protoData = m_protocol->createData(sock);
    sock->setSslConfiguration(m_sslConfiguration);

    connect(sock, &QIODevice::readyRead, this, [sock]() {
        sock->timeout = false;
        sock->proto->parse(sock, sock);
    });
    connect(sock, &SslSocket::finished, this, [this, sock]() {
        sock->deleteLater();
        if (--m_processing == 0) {
            m_engine->stopSocketTimeout();
        }
    });

    if (Q_LIKELY(sock->setSocketDescriptor(
            handle, QTcpSocket::ConnectedState, QTcpSocket::ReadWrite | QTcpSocket::Unbuffered))) {
        sock->proto = m_protocol;

        sock->serverAddress = m_serverAddress;
        sock->remoteAddress = sock->peerAddress();
        sock->remotePort    = sock->peerPort();
        sock->protoData->setupNewConnection(sock);

        for (const auto &opt : m_socketOptions) {
            sock->setSocketOption(opt.first, opt.second);
        }

        if (++m_processing) {
            m_engine->startSocketTimeout();
        }

        sock->startServerEncryption();
        if (m_http2Protocol) {
            connect(sock, &SslSocket::encrypted, this, [this, sock]() {
                if (sock->sslConfiguration().nextNegotiatedProtocol() == "h2") {
                    sock->proto     = m_http2Protocol;
                    sock->protoData = sock->proto->createData(sock);
                }
            });
        }
    } else {
        delete sock;
    }
}

void TcpSslServer::shutdown()
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
                    if (!m_processing) {
                        m_engine->serverShutdown();
                    }
                });
                m_engine->handleSocketShutdown(socket);
            }
        }
    }
}

void TcpSslServer::timeoutConnections()
{
    if (m_processing) {
        const auto childrenL = children();
        for (auto child : childrenL) {
            auto socket = qobject_cast<SslSocket *>(child);
            if (socket && !socket->processing &&
                socket->state() == QAbstractSocket::ConnectedState) {
                if (socket->timeout) {
                    socket->connectionClose();
                } else {
                    socket->timeout = true;
                }
            }
        }
    }
}

void TcpSslServer::setSslConfiguration(const QSslConfiguration &conf)
{
    m_sslConfiguration = conf;
    conf.allowedNextProtocols();
}

void TcpSslServer::setHttp2Protocol(Protocol *protocol)
{
    m_http2Protocol = protocol;
}

#    include "moc_tcpsslserver.cpp"

#endif // QT_NO_SSL
