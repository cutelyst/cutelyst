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
    quint16 remotePort;
    QByteArray remoteUser;
    Engine *engine;
    // Pointer to Engine data
    void *requestPtr = 0;

    // Instead of setting this you might use setPathURIAndQueryParams
    bool https = false;
    QString path;
    QString serverAddress;
    quint16 serverPort;
    QString queryString;

protected:
    friend class Request;
    friend class Dispatcher;

    // Engines don't need to touch this
    QStringList args;

    mutable bool uriParsed = false;
    mutable QUrl uri;

    mutable bool queryParamParsed = false;
    mutable QUrlQuery queryParamUrl;
    mutable QMultiHash<QString, QString> queryParam;

    mutable bool cookiesParsed = false;
    mutable QList<QNetworkCookie> cookies;
    mutable bool bodyParsed = false;
    mutable QMultiHash<QString, QString> bodyParam;
    mutable QMultiHash<QString, QString> param;
    mutable Uploads uploads;
};

}

#endif // CUTELYST_REQUEST_P_H
