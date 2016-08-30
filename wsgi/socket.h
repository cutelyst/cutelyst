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

class QIODevice;

namespace CWSGI {

class CWsgiEngine;
class Socket
{
public:
    Socket();
    virtual ~Socket();

    inline void resetSocket() {
        buffer = QByteArray();

        buf_size = 0;
        connState = 0;
        beginLine = 0;
        last = 0;
        headerClose = 0;
        processing = false;
    }

    Cutelyst::Headers headers;
    QString serverAddress;
    QString method;
    QString path;
    QByteArray query;
    QString protocol;
    QByteArray buffer;
    quint64 start;
    CWsgiEngine *engine;
    char *buf;
    int buf_size = 0;
    int connState = 0;
    int beginLine = 0;
    int last = 0;
    int headerClose = 0;
    bool processing = false;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = 0);

    void socketDisconnected();

Q_SIGNALS:
    void finished();
};

}

#endif // SOCKET_H
