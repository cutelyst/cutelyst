/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PROTOCOLHTTP2_H
#define PROTOCOLHTTP2_H

#include "hpack.h"
#include "protocol.h"
#include "socket.h"

#include <context.h>
#include <enginerequest.h>

#include <QObject>

// namespace Cutelyst {
// class Headers;
// }

class QEventLoop;
namespace Cutelyst {

class H2Frame
{
public:
    quint32 len;
    quint32 streamId;
    quint8 type;
    quint8 flags;
};

class ProtoRequestHttp2;
class H2Stream final : public Cutelyst::EngineRequest
{
public:
    enum State {
        Idle,
        Open,
        HalfClosed,
        Closed
    };
    H2Stream(quint32 streamId, qint32 initialWindowSize, ProtoRequestHttp2 *protoRequestH2);
    ~H2Stream() override;

    virtual qint64 doWrite(const char *data, qint64 len) override final;

    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) override final;

    virtual void processingFinished() override final;

    void windowUpdated();

    QEventLoop *loop = nullptr;
    QString scheme;
    ProtoRequestHttp2 *protoRequest;
    quint32 streamId;
    qint32 windowSize    = 65535;
    qint64 contentLength = -1;
    qint32 dataSent      = 0;
    qint64 consumedData  = 0;
    quint8 state         = Idle;
    bool gotPath         = false;
};

class ProtoRequestHttp2 final : public ProtocolData
{
    Q_GADGET
public:
    ProtoRequestHttp2(Cutelyst::Socket *sock, int bufferSize);
    ~ProtoRequestHttp2() override;

    void setupNewConnection(Cutelyst::Socket *sock) override final;

    inline void resetData() override final
    {
        ProtocolData::resetData();

        stream_id = 0;
        pktsize   = 0;
        delete hpack;
        hpack = nullptr;

        auto it = streams.constBegin();
        while (it != streams.constEnd()) {
            // If we deleteLater the context, there might
            // be an event that tries to finalize the request
            // and it will encounter a null context pointer
            delete it.value()->context;
            delete it.value();
            ++it;
        }
        streams.clear();

        headersBuffer.clear();
        maxStreamId               = 0;
        streamForContinuation     = 0;
        dataSent                  = 0;
        windowSize                = 65535;
        settingsInitialWindowSize = 65535;
        canPush                   = false;
    }

    quint32 stream_id = 0;
    quint32 pktsize   = 0;

    QByteArray headersBuffer;
    HPack *hpack                     = nullptr;
    quint64 streamForContinuation    = 0;
    quint32 maxStreamId              = 0;
    qint32 dataSent                  = 0;
    qint32 windowSize                = 65535;
    qint32 settingsInitialWindowSize = 65535;
    quint32 settingsMaxFrameSize     = 16384;
    quint8 processing                = 0;
    bool canPush                     = true;

    QHash<quint32, H2Stream *> streams;
};

class ProtocolHttp2 final : public Protocol
{
public:
    explicit ProtocolHttp2(Server *wsgi);
    ~ProtocolHttp2() override;

    Type type() const override;

    void parse(Cutelyst::Socket *sock, QIODevice *io) const override final;

    ProtocolData *createData(Cutelyst::Socket *sock) const override final;

    int parseSettings(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseData(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseHeaders(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parsePriority(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parsePing(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseRstStream(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;
    int parseWindowUpdate(ProtoRequestHttp2 *request, QIODevice *io, const H2Frame &fr) const;

    int sendGoAway(QIODevice *io, quint32 lastStreamId, quint32 error) const;
    int sendRstStream(QIODevice *io, quint32 streamId, quint32 error) const;
    int sendSettings(QIODevice *io, const std::vector<std::pair<quint16, quint32>> &settings) const;
    int sendSettingsAck(QIODevice *io) const;
    int sendPing(QIODevice *io, quint8 flags, const char *data = nullptr, qint32 dataLen = 0) const;
    int sendData(QIODevice *io, quint32 streamId, qint32 flags, const char *data, qint32 dataLen) const;
    int sendFrame(QIODevice *io, quint8 type, quint8 flags = 0, quint32 streamId = 0, const char *data = nullptr, qint32 dataLen = 0) const;

    void queueStream(Cutelyst::Socket *socket, H2Stream *stream) const;

    bool upgradeH2C(Cutelyst::Socket *socket, QIODevice *io, const Cutelyst::EngineRequest &request);

public:
    quint32 m_maxFrameSize;
    qint32 m_headerTableSize;
};

} // namespace Cutelyst

#endif // PROTOCOLHTTP2_H
