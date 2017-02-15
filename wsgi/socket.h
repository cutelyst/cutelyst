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
#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QLocalSocket>
#include <QHostAddress>
#include <Cutelyst/Headers>
#include <Cutelyst/Engine>

#include "cwsgiengine.h"

class QIODevice;

namespace CWSGI {

class WSGI;
class Protocol;
class Socket : public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    Socket(WSGI *wsgi);
    virtual ~Socket();

    enum HeaderClose {
        HeaderCloseNotSet = 0,
        HeaderCloseKeep,
        HeaderCloseClose
    };
    Q_ENUM(HeaderClose)

    enum ParserState {
        MethodLine = 0,
        HeaderLine,
        ContentBody
    };
    Q_ENUM(ParserState)

    inline void resetSocket() {
        connState = MethodLine;
        stream_id = 0; //FCGI
        buf_size = 0;
        beginLine = 0;
        last = 0;
        startOfRequest = 0;
        headerClose = HeaderCloseNotSet;
        processing = false;
        headerHost = false;
        timeout = false;
        delete body;
        body = nullptr;
    }

    virtual void connectionClose() = 0;

    qint64 contentLength;
    CWsgiEngine *engine;
    Protocol *proto;
    char *buffer;
    ParserState connState = MethodLine;
    quint64 stream_id = 0;// FGCI
    quint32 buf_size = 0;
    quint32 last = 0;
    int beginLine = 0;
    int headerClose = HeaderCloseNotSet;
    bool headerHost = false;
    bool processing = false;
    bool timeout = false;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(WSGI *wsgi, QObject *parent = 0);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(TcpSocket *bj);
};

class LocalSocket : public QLocalSocket, public Socket
{
    Q_OBJECT
public:
    explicit LocalSocket(WSGI *wsgi, QObject *parent = 0);

    virtual void connectionClose() override;
    void socketDisconnected();

Q_SIGNALS:
    void finished(LocalSocket *bj);
};

}

#endif // SOCKET_H
