/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "protocol.h"

#include "wsgi.h"

using namespace CWSGI;

Protocol::Protocol(WSGI *wsgi)
{
    m_bufferSize = wsgi->bufferSize();
    m_postBuffering = wsgi->postBuffering();
    m_webSocketBufferSize = wsgi->bufferSize();
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

qint64 Protocol::sendBody(QIODevice *io, Socket *sock, const char *data, qint64 len)
{
    Q_UNUSED(sock)
    return io->write(data, len);
}
