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
#include "protocol.h"

#include "socket.h"
#include "wsgi.h"

#include <QTemporaryFile>
#include <QBuffer>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CWSGI_PROTO, "cwsgi.proto")

using namespace CWSGI;

ProtocolData::ProtocolData(Socket *_sock, int bufferSize) : sock(_sock)
    , io(dynamic_cast<QIODevice *>(_sock))
    , buffer(new char[bufferSize])
{
}

ProtocolData::~ProtocolData()
{
    delete [] buffer;
    delete upgradedFrom;
}

Protocol::Protocol(WSGI *wsgi)
{
    m_bufferSize = wsgi->bufferSize();
    m_postBuffering = wsgi->postBuffering();
    m_postBufferSize = qMax(static_cast<qint64>(32), wsgi->postBufferingBufsize());
    m_postBuffer = new char[wsgi->postBufferingBufsize()];
}

Protocol::~Protocol()
{
    delete [] m_postBuffer;
}

Protocol::Type Protocol::type() const
{
    return Unknown;
}

QIODevice *Protocol::createBody(qint64 contentLength) const
{
    QIODevice *body;
    if (m_postBuffering && contentLength > m_postBuffering) {
        auto temp = new QTemporaryFile;
        if (!temp->open()) {
            qCWarning(CWSGI_PROTO) << "Failed to open temporary file to store post" << temp->errorString();
            // On error close connection immediately
            return nullptr;
        }
        body = temp;
    } else if (m_postBuffering && contentLength <= m_postBuffering) {
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(contentLength);
        body = buffer;
    } else {
        // Unbuffered
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(contentLength);
        body = buffer;
    }
    return body;
}

#include "moc_protocol.cpp"
