/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_RESPONSE_P_H
#define CUTELYST_RESPONSE_P_H

#include "response.h"

#include <QtCore/QUrl>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkCookie>

namespace Cutelyst {

class Context;
class Engine;
class EngineRequest;
class ResponsePrivate
{
public:
    inline ResponsePrivate(Context *c, Engine *e, const Headers &h) : headers(h), context(c) { Q_UNUSED(e) }
    inline ResponsePrivate(Context *c, EngineRequest *er, const Headers &h) : headers(h), context(c), engineRequest(er) { }
    inline void setBodyData(const QByteArray &body);

    Headers headers;
    QMap<QByteArray, QNetworkCookie> cookies;
    QByteArray bodyData;
    QUrl location;
    QIODevice *bodyIODevice = nullptr;
    Context *context;
    EngineRequest *engineRequest;
    quint16 status = Response::OK;
};

}

#endif // CUTELYST_RESPONSE_P_H
