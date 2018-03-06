/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef PROTOCOLFASTCGI_H
#define PROTOCOLFASTCGI_H

#include <QObject>

#include "protocol.h"
#include "socket.h"

namespace CWSGI {

class WSGI;
class ProtoRequestFastCGI : public ProtocolData, public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    ProtoRequestFastCGI(WSGI *wsgi, Cutelyst::Engine *_engine);
    virtual ~ProtoRequestFastCGI();

protected:
    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    virtual qint64 doWrite(const char *data, qint64 len) override final;

    inline qint64 doWrite(const QByteArray &data) {
        return doWrite(data.constData(), data.size());
    }

public:
    quint64 stream_id = 0;
};

class ProtocolFastCGI : public Protocol
{
public:
    ProtocolFastCGI(WSGI *wsgi);
    virtual ~ProtocolFastCGI();

    virtual Type type() const override;

    inline qint64 readBody(Socket *socket, QIODevice *io, qint64 bytesAvailable) const;
    virtual void readyRead(Socket *sock, QIODevice *io) const override;
    virtual bool sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers) override;
    qint64 sendBody(QIODevice *io, Socket *sock, const char *data, qint64 len) override;

private:
    inline quint16 addHeader(ProtoRequestFastCGI *request, const char *key, quint16 keylen, const char *val, quint16 vallen) const;
    inline int parseHeaders(ProtoRequestFastCGI *request, const char *buf, size_t len) const;
    inline int processPacket(ProtoRequestFastCGI *request) const;
    inline bool writeBody(ProtoRequestFastCGI *request, char *buf, qint64 len) const;
    // write a STDOUT packet
    int wsgi_proto_fastcgi_write(QIODevice *io, ProtoRequestFastCGI *request, const char *buf, int len);
};

}

#endif // PROTOCOLFASTCGI_H
