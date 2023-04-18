/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDebug>
#include <QObject>

class QIODevice;

namespace Cutelyst {

class Server;
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

    inline virtual void resetData()
    {
        connState         = MethodLine;
        buf_size          = 0;
        headerConnection  = HeaderConnectionNotSet;
        headerHost        = false;
        X_Forwarded_For   = false;
        X_Forwarded_Host  = false;
        X_Forwarded_Proto = false;
    }

    virtual void socketDisconnected() {}
    virtual void setupNewConnection(Socket *sock) = 0;

    qint64 contentLength = 0;
    Socket *sock; // temporary
    QIODevice *io;
    ProtocolData *upgradedFrom        = nullptr;
    int buf_size                      = 0;
    ParserState connState             = MethodLine;
    HeaderConnection headerConnection = HeaderConnectionNotSet;
    char *buffer;
    bool headerHost        = false;
    bool X_Forwarded_For   = false;
    bool X_Forwarded_Host  = false;
    bool X_Forwarded_Proto = false;
};

class Protocol
{
    Q_GADGET
public:
    enum class Type {
        Unknown,
        Http11,
        Http11Websocket,
        Http2,
        FastCGI1
    };
    Q_ENUM(Type)

    Protocol(Server *wsgi);
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

inline quint64 net_be64(const char *buf)
{
    quint64 ret = 0;
    auto src    = reinterpret_cast<const quint64 *>(buf);
    auto ptr    = reinterpret_cast<quint8 *>(&ret);
    ptr[0]      = static_cast<quint8>((*src >> 56) & 0xff);
    ptr[1]      = static_cast<quint8>((*src >> 48) & 0xff);
    ptr[2]      = static_cast<quint8>((*src >> 40) & 0xff);
    ptr[3]      = static_cast<quint8>((*src >> 32) & 0xff);
    ptr[4]      = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[5]      = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[6]      = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[7]      = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint32 net_be32(const char *buf)
{
    quint32 ret = 0;
    auto src    = reinterpret_cast<const quint32 *>(buf);
    auto ptr    = reinterpret_cast<quint8 *>(&ret);
    ptr[0]      = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[1]      = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[2]      = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[3]      = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint32 net_be24(const char *buf)
{
    quint32 ret = 0;
    auto src    = reinterpret_cast<const quint32 *>(buf);
    auto ptr    = reinterpret_cast<quint8 *>(&ret);
    ptr[0]      = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[1]      = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[2]      = static_cast<quint8>(*src & 0xff);
    return ret;
}

inline quint16 net_be16(const char *buf)
{
    quint16 ret = 0;
    auto src    = reinterpret_cast<const quint16 *>(buf);
    auto ptr    = reinterpret_cast<quint8 *>(&ret);
    ptr[0]      = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[1]      = static_cast<quint8>(*src & 0xff);
    return ret;
}

} // namespace Cutelyst

#endif // PROTOCOL_H
