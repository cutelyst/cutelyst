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
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

class QIODevice;

namespace CWSGI {

class WSGI;
class Socket;
class Protocol;
class ProtocolData
{
    Q_GADGET
public:
    ProtocolData(Socket *sock, int bufferSize);
    virtual ~ProtocolData();

    enum HeaderConnection {
        HeaderConnectionNotSet = 0,
        HeaderConnectionKeep,
        HeaderConnectionClose,
        HeaderConnectionUpgrade
    };
    Q_ENUM(HeaderConnection)

    enum ParserState {
        MethodLine = 0,
        HeaderLine,
        ContentBody,
        H2Frames
    };
    Q_ENUM(ParserState)

    inline virtual void resetSocket() {
        connState = MethodLine;
        buf_size = 0;
        headerConnection = HeaderConnectionNotSet;
        headerHost = false;
    }

    virtual void socketDisconnected() {}

    qint64 contentLength;
    Socket *sock;//temporary
    QIODevice *io;
    quint32 buf_size = 0;
    ParserState connState = MethodLine;
    HeaderConnection headerConnection = HeaderConnectionNotSet;
    char *buffer;
    bool headerHost = false;
};

class Protocol
{
public:
    enum Type {
        Unknown,
        Http11,
        Http2,
        FastCGI1
    };

    Protocol(WSGI *wsgi);
    virtual ~Protocol();

    virtual Type type() const;

    virtual void parse(Socket *sock, QIODevice *io) const = 0;

    virtual ProtocolData *createData(Socket *sock) const = 0;

    qint64 m_postBufferSize;
    qint64 m_bufferSize;
    qint64 m_postBuffering;
    char *m_postBuffer;
};

}

#endif // PROTOCOL_H
