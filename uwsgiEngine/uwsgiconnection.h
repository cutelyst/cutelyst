/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
