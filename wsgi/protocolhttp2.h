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

namespace CWSGI {

class H2Frame
{
public:
    quint32 len;
    quint32 streamId;
    quint8 type;
    quint8 flags;
};

class HuffmanTree;
class ProtocolHttp2 : public Protocol
{
public:
    explicit ProtocolHttp2(WSGI *wsgi);
    ~ProtocolHttp2();

    virtual Type type() const override;

    virtual void readyRead(Socket *sock, QIODevice *io) const override;

    int parseSettings(Socket *sock, QIODevice *io, const H2Frame &fr) const;
    int parseData(Socket *sock, QIODevice *io, const H2Frame &fr) const;
    int parseHeaders(Socket *sock, QIODevice *io, const H2Frame &fr) const;
    int parsePriority(Socket *sock, QIODevice *io, const H2Frame &fr) const;
    int parsePing(Socket *sock, QIODevice *io, const H2Frame &fr) const;
    int parseRstStream(Socket *sock, QIODevice *io, const H2Frame &fr) const;

    int sendGoAway(QIODevice *io, quint32 error) const;
    int sendSettings(QIODevice *io, const std::vector<std::pair<quint16, quint32> > &settings) const;
    int sendSettingsAck(QIODevice *io) const;
    int sendPing(QIODevice *io, quint8 flags, const char *data = nullptr, qint32 dataLen = 0) const;
    int sendFrame(QIODevice *io, quint8 type, quint8 flags = 0, quint32 streamId = 0, const char *data = nullptr, qint32 dataLen = 0) const;
    virtual bool sendHeaders(QIODevice *io, CWSGI::Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers) override;

    quint32 m_maxFrameSize;
    HuffmanTree *hTree;
};

}

#endif // PROTOCOLHTTP2_H
