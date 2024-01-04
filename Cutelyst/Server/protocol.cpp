/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "protocol.h"

#include "server.h"
#include "socket.h"

#include <Cutelyst/cutelyst_global.h>

#include <QBuffer>
#include <QLoggingCategory>
#include <QTemporaryFile>

Q_LOGGING_CATEGORY(C_SERVER_PROTO, "cutelyst.server.proto", QtWarningMsg)
#if defined(Cutelyst4Qt6Server_EXPORTS)
Q_LOGGING_CATEGORY(CUTELYST_STATS, "cutelyst.stats", QtWarningMsg)
#else
Q_DECLARE_LOGGING_CATEGORY(CUTELYST_STATS)
#endif

using namespace Cutelyst;

ProtocolData::ProtocolData(Cutelyst::Socket *_sock, int bufferSize)
    : sock(_sock)
    , io(dynamic_cast<QIODevice *>(_sock))
    , buffer(new char[bufferSize])
{
}

ProtocolData::~ProtocolData()
{
    delete[] buffer;
}

Cutelyst::Protocol::Protocol(Cutelyst::Server *server)
    : m_postBufferSize{qMax(static_cast<qint64>(32), server->postBufferingBufsize())}
    , m_postBuffering{server->postBuffering()}
    , m_postBuffer{new char[server->postBufferingBufsize()]}
    , m_bufferSize{server->bufferSize()}
    , useStats{CUTELYST_STATS().isDebugEnabled()}
{
}

Cutelyst::Protocol::~Protocol()
{
    delete[] m_postBuffer;
}

Cutelyst::Protocol::Type Cutelyst::Protocol::type() const
{
    return Protocol::Type::Unknown;
}

QIODevice *Cutelyst::Protocol::createBody(qint64 contentLength) const
{
    QIODevice *body;
    if (m_postBuffering && contentLength > m_postBuffering) {
        auto temp = new QTemporaryFile;
        if (!temp->open()) {
            qCWarning(C_SERVER_PROTO)
                << "Failed to open temporary file to store post" << temp->errorString();
            // On error close connection immediately
            return nullptr;
        }
        body = temp;
    } else if (m_postBuffering && contentLength <= m_postBuffering) {
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(int(contentLength));
        body = buffer;
    } else {
        // Unbuffered
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(int(contentLength));
        body = buffer;
    }
    return body;
}

#include "moc_protocol.cpp"
