/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "socket.h"

#include "server.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_SOCK, "cwsgi.socket", QtWarningMsg)

using namespace Cutelyst;

Socket::Socket(bool secure, Cutelyst::Engine *_engine)
    : engine(_engine)
    , isSecure(secure)
{
}

Socket::~Socket()
{
    delete protoData;
}

TcpSocket::TcpSocket(Cutelyst::Engine *engine, QObject *parent)
    : QTcpSocket(parent)
    , Socket(false, engine)
{
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected, Qt::DirectConnection);
}

void TcpSocket::connectionClose()
{
    QTcpSocket::flush();
    disconnectFromHost();
}

bool TcpSocket::requestFinished()
{
    bool disconnected = state() != ConnectedState;
    if (!--processing && disconnected) {
        Q_EMIT finished();
    }
    return !disconnected;
}

bool TcpSocket::flush()
{
    return QTcpSocket::flush();
}

void TcpSocket::socketDisconnected()
{
    if (!processing) {
        Q_EMIT finished();
    } else {
        protoData->socketDisconnected();
    }
}

LocalSocket::LocalSocket(Cutelyst::Engine *engine, QObject *parent)
    : QLocalSocket(parent)
    , Socket(false, engine)
{
    connect(this, &QLocalSocket::disconnected, this, &LocalSocket::socketDisconnected, Qt::DirectConnection);
}

void LocalSocket::connectionClose()
{
    QLocalSocket::flush();
    disconnectFromServer();
}

bool LocalSocket::requestFinished()
{
    bool disconnected = state() != ConnectedState;
    if (!--processing && disconnected) {
        Q_EMIT finished();
    }
    return !disconnected;
}

bool LocalSocket::flush()
{
    return QLocalSocket::flush();
}

void LocalSocket::socketDisconnected()
{
    if (!processing) {
        Q_EMIT finished();
    } else {
        protoData->socketDisconnected();
    }
}

#ifndef QT_NO_SSL

SslSocket::SslSocket(Cutelyst::Engine *engine, QObject *parent)
    : QSslSocket(parent)
    , Socket(true, engine)
{
    connect(this, &QSslSocket::disconnected, this, &SslSocket::socketDisconnected, Qt::DirectConnection);
}

void SslSocket::connectionClose()
{
    QSslSocket::flush();
    disconnectFromHost();
}

bool SslSocket::requestFinished()
{
    bool disconnected = state() != ConnectedState;
    if (!--processing && disconnected) {
        Q_EMIT finished();
    }
    return !disconnected;
}

bool SslSocket::flush()
{
    return QSslSocket::flush();
}

void SslSocket::socketDisconnected()
{
    if (!processing) {
        Q_EMIT finished();
    } else {
        protoData->socketDisconnected();
    }
}

#endif // QT_NO_SSL

#include "moc_socket.cpp"
