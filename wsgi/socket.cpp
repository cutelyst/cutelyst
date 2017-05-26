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
#include "socket.h"

#include "wsgi.h"

#include <Cutelyst/Context>

#include <QCoreApplication>
#include <QDebug>

using namespace CWSGI;

TcpSocket::TcpSocket(WSGI *wsgi, QObject *parent) : QTcpSocket(parent), Socket(wsgi)
{
    isSecure = false;
    requestPtr = this;
    startOfRequest = 0;
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected, Qt::DirectConnection);
}

void TcpSocket::connectionClose()
{
    disconnectFromHost();
}

Socket::Socket(WSGI *wsgi)
{
    body = nullptr;
    buffer = new char[wsgi->bufferSize()];
}

Socket::~Socket()
{
    delete [] buffer;
}

void TcpSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

LocalSocket::LocalSocket(WSGI *wsgi, QObject *parent) : QLocalSocket(parent), Socket(wsgi)
{
    isSecure = false;
    requestPtr = this;
    startOfRequest = 0;
    connect(this, &QLocalSocket::disconnected, this, &LocalSocket::socketDisconnected, Qt::DirectConnection);
}

void LocalSocket::connectionClose()
{
    flush();
    disconnectFromServer();
}

void LocalSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

SslSocket::SslSocket(WSGI *wsgi, QObject *parent) : QSslSocket(parent), Socket(wsgi)
{
    isSecure = true;
    requestPtr = this;
    startOfRequest = 0;
    connect(this, &QSslSocket::disconnected, this, &SslSocket::socketDisconnected, Qt::DirectConnection);
}

void SslSocket::connectionClose()
{
    disconnectFromHost();
}

void SslSocket::socketDisconnected()
{
    if (websocketContext) {
        if (websocket_finn_opcode != 0x88) {
            websocketContext->request()->webSocketClosed(1005, QString());
        }

        delete websocketContext;
        websocketContext = nullptr;
    }

    if (!processing) {
        Q_EMIT finished(this);
    }
}

#include "moc_socket.cpp"
