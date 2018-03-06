/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "protocolwebsocket.h"

#include <typeinfo>

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QCoreApplication>
#include <QDebug>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_SOCK, "cwsgi.socket")

using namespace CWSGI;

TcpSocket::TcpSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QTcpSocket(parent), Socket(wsgi, engine)
{
//    io = this;
//    isSecure = false;
//    startOfRequest = 0;
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected, Qt::DirectConnection);
}

void TcpSocket::connectionClose()
{
    flush();
    disconnectFromHost();
}

Socket::Socket(WSGI *wsgi, Cutelyst::Engine *_engine)
{
//    body = nullptr;
//    buffer = new char[wsgi->bufferSize()];
}

Socket::~Socket()
{
//    delete [] buffer;
}

ProtocolData::ProtocolData()
{
}

ProtocolData::~ProtocolData()
{

}

//qint64 ProtocolData::doWrite(const char *data, qint64 len)
//{
//    qint64 ret = sock->proto->sendBody(io, sock, data, len);
//    return ret;
//}

//bool ProtocolData::writeHeaders(quint16 status, const Cutelyst::Headers &headers)
//{
//    return sock->proto->sendHeaders(io, sock, status, static_cast<CWsgiEngine *>(sock->engine)->lastDate(), headers);
//}

void TcpSocket::socketDisconnected()
{
//    if (websocketContext) {
//        if (websocket_finn_opcode != 0x88) {
//            websocketContext->request()->webSocketClosed(1005, QString());
//        }

//        delete websocketContext;
//        websocketContext = nullptr;
//    }

//    if (!processing) {
//        Q_EMIT finished(this);
//    }
}

LocalSocket::LocalSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QLocalSocket(parent), Socket(wsgi, engine)
{
//    io = this;
//    isSecure = false;
//    startOfRequest = 0;
    connect(this, &QLocalSocket::disconnected, this, &LocalSocket::socketDisconnected, Qt::DirectConnection);
}

void LocalSocket::connectionClose()
{
    flush();
    disconnectFromServer();
}

void LocalSocket::socketDisconnected()
{
//    if (websocketContext) {
//        if (websocket_finn_opcode != 0x88) {
//            websocketContext->request()->webSocketClosed(1005, QString());
//        }

//        delete websocketContext;
//        websocketContext = nullptr;
//    }

//    if (!processing) {
//        Q_EMIT finished(this);
//    }
}

SslSocket::SslSocket(WSGI *wsgi, Cutelyst::Engine *engine, QObject *parent) : QSslSocket(parent), Socket(wsgi, engine)
{
//    io = this;
//    isSecure = true;
//    startOfRequest = 0;
    connect(this, &QSslSocket::disconnected, this, &SslSocket::socketDisconnected, Qt::DirectConnection);
}

void SslSocket::connectionClose()
{
    flush();
    disconnectFromHost();
}

void SslSocket::socketDisconnected()
{
//    if (websocketContext) {
//        if (websocket_finn_opcode != 0x88) {
//            websocketContext->request()->webSocketClosed(1005, QString());
//        }

//        delete websocketContext;
//        websocketContext = nullptr;
//    }

//    if (!processing) {
//        Q_EMIT finished(this);
//    }
}

#include "moc_socket.cpp"
