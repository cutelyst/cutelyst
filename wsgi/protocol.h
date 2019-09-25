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
#include <QDebug>

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

    inline virtual void resetData() {
        connState = MethodLine;
        buf_size = 0;
        headerConnection = HeaderConnectionNotSet;
        headerHost = false;
        X_Forwarded_For = false;
        X_Forwarded_Host = false;
        X_Forwarded_Proto = false;
    }

    virtual void socketDisconnected() {}
    virtual void setupNewConnection(Socket *sock) = 0;

    qint64 contentLength = 0;
    Socket *sock;//temporary
    QIODevice *io;
    ProtocolData *upgradedFrom = nullptr;
    int buf_size = 0;
    ParserState connState = MethodLine;
    HeaderConnection headerConnection = HeaderConnectionNotSet;
    char *buffer;
    bool headerHost = false;
    bool X_Forwarded_For = false;
    bool X_Forwarded_Host = false;
    bool X_Forwarded_Proto = false;
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

    QIODevice *createBody(qint64 contentLength) const;

    qint64 m_postBufferSize;
    qint64 m_postBuffering;
    int m_bufferSize;
    char *m_postBuffer;
};

inline quint64 net_be64(const char *buf) {
    quint64 ret = 0;
    auto src = reinterpret_cast<const quint64 *>(buf);
    auto ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 56) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 48) & 0xff);
    ptr[2] = static_cast<quint8>((*src >> 40) & 0xff);
    ptr[3] = static_cast<quint8>((*src >> 32) & 0xff);
    ptr[4] = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[5] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[6] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[7] = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint32 net_be32(const char *buf) {
    quint32 ret = 0;
    auto src = reinterpret_cast<const quint32 *>(buf);
    auto ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[2] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[3] = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint32 net_be24(const char *buf) {
    quint32 ret = 0;
    auto src = reinterpret_cast<const quint32 *>(buf);
    auto ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[2] = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint16 net_be16(const char *buf) {
    quint16 ret = 0;
    auto src = reinterpret_cast<const quint16 *>(buf);
    auto ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[1] = static_cast<quint8>(*src & 0xff);
    return ret;
}

}

#endif // PROTOCOL_H
