/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_REQUEST_P_H
#define CUTELYST_REQUEST_P_H

#include "request.h"

#include "upload.h"

#include <QStringList>
#include <QHostAddress>
#include <QUrlQuery>
#include <QUrl>

namespace Cutelyst {

typedef QPair<QString, QString> StringPair;

class Engine;
class RequestPrivate
{
public:
    // call reset before reusing it
    void reset();

    void parseUrlQuery() const;
    void parseBody() const;
    void parseCookies() const;

    // Manually filled by the Engine
    QByteArray method;
    QByteArray protocol;
    Headers headers;
    QIODevice *body = 0;
    QHostAddress remoteAddress;
    mutable QString remoteHostname;
    quint16 remotePort;
    QByteArray remoteUser;
    Engine *engine;
    // Pointer to Engine data
    void *requestPtr = 0;

    // Instead of setting this you might use setPathURIAndQueryParams
    bool https = false;
    QByteArray path;
    QString query;
    QString serverAddress;
    quint16 serverPort;

protected:
    friend class Request;
    friend class Dispatcher;
    friend class DispatchType;

    // Engines don't need to touch this
    QStringList args;
    QByteArray match;

    mutable bool urlParsed = false;
    mutable QUrl url;

    mutable bool cookiesParsed = false;
    mutable QList<QNetworkCookie> cookies;

    mutable bool queryParamParsed = false;
    mutable CQueryMultiMap queryParam;

    mutable bool bodyParsed = false;
    mutable CQueryMultiMap bodyParam;

    mutable bool paramParsed = false;
    mutable CQueryMultiMap param;

    mutable Uploads uploads;
};

}

#endif // CUTELYST_REQUEST_P_H
