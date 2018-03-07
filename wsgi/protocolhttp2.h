/*
 * Copyright (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef PROTOCOLHTTP2_H
#define PROTOCOLHTTP2_H

#include <QObject>

#include "protocol.h"
#include "socket.h"
#include "hpack.h"

namespace CWSGI {

class H2Frame
{
public:
    quint32 len;
    quint32 streamId;
    quint8 type;
    quint8 flags;
};

class H2Stream
{
public:
    enum state {
        Idle,
        Open,
        HalfClosed,
        Closed
    };

    QString method;
    QString path;
    QString scheme;
    QString authority;
    Cutelyst::Headers headers;
    quint32 streamId;
    quint32 windowSize = 65535;
    qint64 contentLength = -1;
    qint64 consumedData = 0;
    quint8 state = Idle;
};

class ProtoRequestHttp2 : public ProtocolData
{
    Q_GADGET
public:
    ProtoRequestHttp2(Socket *sock, int bufferSize);
    virtual ~ProtoRequestHttp2();

    inline virtual void resetSocket() override final {
        ProtocolData::resetSocket();

        stream_id = 0;
        pktsize = 0;
        delete hpack;
        hpack = nullptr;
        qDeleteAll(streams);
        streams.clear();
        maxStreamId = 0;
        streamForContinuation = 0;
        windowSize = 65535;
        canPush = false;
    }
    quint64 stream_id = 0;
    quint32 pktsize = 0;

    HPack *hpack = nullptr;
    quint64 maxStreamId = 0;
    quint64 streamForContinuation = 0;
    quint32 windowSize = 65535;
    bool canPush = false;

    QHash<quint32, H2Stream *> streams;
};

class HPack;
class ProtocolHttp2 : public Protocol
{
public:
    explicit ProtocolHttp2(WSGI *wsgi);
    ~ProtocolHttp2();

    virtual Type type() const override;

    virtual void parse(Socket *sock, QIODevice *io) const override final;

    virtual ProtocolData *createData(Socket *sock) const override final;

    int parseSettings(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseData(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseHeaders(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parsePriority(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parsePing(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseRstStream(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseWindowUpdate(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;

    int sendGoAway(QIODevice *io, quint32 lastStreamId, quint32 error) const;
    int sendRstStream(QIODevice *io, quint32 streamId, quint32 error) const;
    int sendSettings(QIODevice *io, const std::vector<std::pair<quint16, quint32> > &settings) const;
    int sendSettingsAck(QIODevice *io) const;
    int sendPing(QIODevice *io, quint8 flags, const char *data = nullptr, qint32 dataLen = 0) const;
    int sendData(QIODevice *io, quint32 streamId, qint32 windowSize, const char *data, qint32 dataLen) const;
    int sendFrame(QIODevice *io, quint8 type, quint8 flags = 0, quint32 streamId = 0, const char *data = nullptr, qint32 dataLen = 0) const;

    void sendDummyReply(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;

    quint32 m_maxFrameSize;
    quint32 m_headerTableSize;
};

}

#endif // PROTOCOLHTTP2_H
