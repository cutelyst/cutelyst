/*
 * SPDX-FileCopyrightText: (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PROTOCOLFASTCGI_H
#define PROTOCOLFASTCGI_H

#include "protocol.h"
#include "socket.h"

#include <Cutelyst/Context>

#include <QObject>

namespace Cutelyst {

class Server;
class ProtoRequestFastCGI final : public ProtocolData
    , public Cutelyst::EngineRequest
{
    Q_GADGET
public:
    ProtoRequestFastCGI(Socket *sock, int bufferSize);
    ~ProtoRequestFastCGI() override;

    void setupNewConnection(Socket *sock) override;

    bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    qint64 doWrite(const char *data, qint64 len) override final;

    inline qint64 doWrite(const QByteArray &data)
    {
        return doWrite(data.constData(), data.size());
    }

    void processingFinished() override final;

    inline void resetData() override final
    {
        ProtocolData::resetData();

        // EngineRequest
        // If we deleteLater the context, there might
        // be an event that tries to finalize the request
        // and it will encounter a null context pointer
        delete context;
        context = nullptr;
        body    = nullptr;

        elapsed.invalidate();
        status = InitialState;

        stream_id = 0;
        pktsize   = 0;
    }

public:
    quint16 stream_id = 0;
    quint16 pktsize   = 0;
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

} // namespace Cutelyst

#endif // PROTOCOLFASTCGI_H
