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

#include "cutelystrequest_p.h"
#include "cutelystengine.h"

// TODO make this configurable
#define TIMEOUT 3000

CutelystRequest::CutelystRequest() :
    d_ptr(new CutelystRequestPrivate)
{
}

CutelystRequest::CutelystRequest(CutelystRequestPrivate *prv) :
    d_ptr(prv)
{

}

CutelystRequest::~CutelystRequest()
{
}

QHostAddress CutelystRequest::peerAddress() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerAddress();
}

QString CutelystRequest::peerName() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerName();
}

quint16 CutelystRequest::peerPort() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerPort();
}

QString CutelystRequest::path() const
{
    Q_D(const CutelystRequest);
    return d->url.path();
}

QStringList CutelystRequest::args() const
{
    Q_D(const CutelystRequest);
    return d->args;
}

QString CutelystRequest::base() const
{

}

QString CutelystRequest::body() const
{

}

QMultiHash<QString, QString> CutelystRequest::bodyParameters() const
{
    Q_D(const CutelystRequest);
    return d->bodyParam;
}

QMultiHash<QString, QString> CutelystRequest::bodyParam() const
{
    return bodyParameters();
}

QMultiHash<QString, QString> CutelystRequest::queryParameters() const
{
    Q_D(const CutelystRequest);
    return d->queryParam;
}

QMultiHash<QString, QString> CutelystRequest::queryParam() const
{
    return queryParameters();
}

QMultiHash<QString, QString> CutelystRequest::parameters() const
{
    Q_D(const CutelystRequest);
    return d->bodyParam + d->queryParam;
}

QMultiHash<QString, QString> CutelystRequest::param() const
{
    return parameters();
}

QNetworkCookie CutelystRequest::cookie(const QByteArray &name) const
{
    Q_D(const CutelystRequest);
    foreach (const QNetworkCookie &cookie, d->cookies) {
        if (cookie.name() == name) {
            return cookie;
        }
    }
    return QNetworkCookie();
}

QList<QNetworkCookie> CutelystRequest::cookies() const
{
    Q_D(const CutelystRequest);
    return d->cookies;
}

QString CutelystRequest::header(const QString &key) const
{
    Q_D(const CutelystRequest);
    return d->headers.value(key);
}

QHash<QString, QByteArray> CutelystRequest::headers() const
{
    Q_D(const CutelystRequest);
    return d->headers;
}

QString CutelystRequest::method() const
{
    Q_D(const CutelystRequest);
    return d->method;
}

void CutelystRequest::setArgs(const QStringList &args)
{
    Q_D(CutelystRequest);
    d->args = args;
}
