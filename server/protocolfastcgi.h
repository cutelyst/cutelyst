/*
 * Copyright (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include <Cutelyst/Context>

#include "protocol.h"
#include "socket.h"

namespace Cutelyst {

class Server;
class ProtoRequestFastCGI final : public ProtocolData, public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    ProtoRequestFastCGI(Socket *sock, int bufferSize);
    ~ProtoRequestFastCGI() override;

    void setupNewConnection(Socket *sock) override;

    bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    qint64 doWrite(const char *data, qint64 len) override final;

    inline qint64 doWrite(const QByteArray &data) {
        return doWrite(data.constData(), data.size());
    }

    void processingFinished() override final;

    inline void resetData() override final {
        ProtocolData::resetData();

        // EngineRequest
        if (status & EngineRequest::Async) {
            context->deleteLater();
        } else {
            delete context;
        }
        context = nullptr;
        body = nullptr;

        elapsed.invalidate();
        status = InitialState;

        stream_id = 0;
        pktsize = 0;
    }

public:
    quint16 stream_id = 0;
    quint16 pktsize = 0;
};

class ProtocolFastCGI final : public Protocol
{
public:
    ProtocolFastCGI(Server *wsgi);
    ~ProtocolFastCGI() override;

    Type type() const override;

    inline qint64 readBody(Socket *socket, QIODevice *io, qint64 bytesAvailable) const;
    void parse(Socket *sock, QIODevice *io) const override final;

    ProtocolData *createData(Socket *sock) const override final;

private:
    inline quint16 addHeader(ProtoRequestFastCGI *request, const char *key, quint16 keylen, const char *val, quint16 vallen) const;
    inline int parseHeaders(ProtoRequestFastCGI *request, const char *buf, quint16 len) const;
    inline int processPacket(ProtoRequestFastCGI *request) const;
    inline bool writeBody(ProtoRequestFastCGI *request, char *buf, qint64 len) const;
};

}

#endif // PROTOCOLFASTCGI_H
