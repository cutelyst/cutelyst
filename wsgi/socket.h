/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include <QHostAddress>
#include <Cutelyst/Headers>
#include <Cutelyst/Engine>

class QIODevice;

namespace CWSGI {

class WSGI;
class CWsgiEngine;
class Socket : public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    Socket(WSGI *wsgi);
    virtual ~Socket();

    enum ParserState {
        MethodLine = 0,
        HeaderLine,
        ContentBody
    };
    Q_ENUM(ParserState)

    inline void resetSocket() {
        connState = MethodLine;
        buf_size = 0;
        beginLine = 0;
        last = 0;
        startOfRequest = 0;
        headerClose = 0;
        processing = false;
        headerHost = false;
        delete body;
        body = nullptr;
    }

    qint64 contentLength;
    CWsgiEngine *engine;
    char *buf;
    ParserState connState = MethodLine;
    int buf_size = 0;
    int beginLine = 0;
    int last = 0;
    int headerClose = 0;
    bool headerHost = false;
    bool processing = false;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(WSGI *wsgi, QObject *parent = 0);

    void socketDisconnected();

Q_SIGNALS:
    void finished();
};

}

#endif // SOCKET_H
