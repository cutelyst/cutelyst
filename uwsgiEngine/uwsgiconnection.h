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
#ifndef UWSGICONNECTION_H
#define UWSGICONNECTION_H

#include <QObject>

#include "Cutelyst/enginerequest.h"

namespace Cutelyst {
class Context;
class Headers;
}

struct wsgi_request;

class uwsgiConnection : public Cutelyst::EngineRequest
{
public:
    explicit uwsgiConnection(wsgi_request *req);

    virtual ~uwsgiConnection();

protected:
    virtual qint64 doWrite(const char *data, qint64 len) final;

    virtual bool writeHeaders(quint16 status, const Cutelyst::Headers &headers) final;

private:
    wsgi_request *request;
};

#endif // UWSGICONNECTION_H
