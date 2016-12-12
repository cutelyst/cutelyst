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
#ifndef PROTOCOLHTTP_H
#define PROTOCOLHTTP_H

#include <QObject>
#include <QByteArrayMatcher>

#include "protocol.h"

namespace CWSGI {

class WSGI;
class Socket;
class ProtocolHttp : public Protocol
{
    Q_OBJECT
public:
    explicit ProtocolHttp(TcpSocket *sock, WSGI *wsgi, QObject *parent = 0);
    ~ProtocolHttp();

    virtual void readyRead();

private:
    inline bool processRequest(TcpSocket *sock);
    inline void parseMethod(const char *ptr, const char *end, Socket *sock);
    inline void parseHeader(const char *ptr, const char *end, Socket *sock);

    QByteArrayMatcher m_matcher = QByteArrayMatcher("\r\n");
    qint64 m_postBufferSize;
    qint64 m_bufferSize;
    qint64 m_postBuffering;
    char *m_postBuffer;
};

}

#endif // PROTOCOLHTTP_H
