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

class ProtocolHttp2 : public Protocol
{
public:
    explicit ProtocolHttp2(WSGI *wsgi);
    ~ProtocolHttp2();

    virtual Type type() const override;

    virtual void readyRead(Socket *sock, QIODevice *io) const override;
    virtual bool sendHeaders(QIODevice *io, CWSGI::Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers) override;
};

}

#endif // PROTOCOLHTTP2_H
