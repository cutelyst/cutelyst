/*
 * Copyright (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "socket.h"

#include "wsgi.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_SOCK, "cwsgi.socket", QtWarningMsg)

using namespace CWSGI;

Socket::Socket(bool secure, Cutelyst::Engine *_engine) : engine(_engine), isSecure(secure)
{

}

Socket::~Socket()
{
    delete protoData;
}

TcpSocket::TcpSocket(Cutelyst::Engine *engine, QObject *parent) : QTcpSocket(parent), Socket(false, engine)
{
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected, Qt::DirectConnection);
}

void TcpSocket::connectionClose()
{
    QTcpSocket::flush();
    disconnectFromHost();
}

void TcpSocket::requestFinished()
{
    if (!--processing && state() != ConnectedState) {
        Q_EMIT finished();
    }
}

void TcpSocket::socketDisconnected()
{
    protoData->socketDisconnected();

    if (!processing) {
        Q_EMIT finished();
    }
}

LocalSocket::LocalSocket(Cutelyst::Engine *engine, QObject *parent) : QLocalSocket(parent), Socket(false, engine)
{
    connect(this, &QLocalSocket::disconnected, this, &LocalSocket::socketDisconnected, Qt::DirectConnection);
}

void LocalSocket::connectionClose()
{
    QLocalSocket::flush();
    disconnectFromServer();
}

void LocalSocket::requestFinished()
{
    if (!--processing && state() != ConnectedState) {
        Q_EMIT finished();
    }
}

void LocalSocket::socketDisconnected()
{
    protoData->socketDisconnected();

    if (!processing) {
        Q_EMIT finished();
    }
}

SslSocket::SslSocket(Cutelyst::Engine *engine, QObject *parent) : QSslSocket(parent), Socket(true, engine)
{
    connect(this, &QSslSocket::disconnected, this, &SslSocket::socketDisconnected, Qt::DirectConnection);
}

void SslSocket::connectionClose()
{
    QSslSocket::flush();
    disconnectFromHost();
}

void SslSocket::requestFinished()
{
    if (!--processing && state() != ConnectedState) {
        Q_EMIT finished();
    }
}

void SslSocket::socketDisconnected()
{
    protoData->socketDisconnected();

    if (!processing) {
        Q_EMIT finished();
    }
}

#include "moc_socket.cpp"
