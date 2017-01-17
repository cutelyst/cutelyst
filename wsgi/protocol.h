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
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

#include "cwsgiengine.h"

namespace CWSGI {

class WSGI;
class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(WSGI *wsgi, QObject *parent = nullptr);
    ~Protocol();

    virtual void readyRead(Socket *sock, QIODevice *io) const = 0;
    virtual bool sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Headers &headers) = 0;
    virtual qint64 sendBody(QIODevice *io, Socket *sock, const char *data, qint64 len);

    WSGI *m_wsgi;
    qint64 m_postBufferSize;
    qint64 m_bufferSize;
    qint64 m_postBuffering;
    char *m_postBuffer;
};

}

#endif // PROTOCOL_H
