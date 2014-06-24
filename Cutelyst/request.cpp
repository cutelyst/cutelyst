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

#include "request_p.h"
#include "engine.h"

// TODO make this configurable
#define TIMEOUT 3000

using namespace Cutelyst;

Request::Request(RequestPrivate *prv) :
    d_ptr(prv)
{

}

Request::~Request()
{
    Q_D(Request);
    delete d_ptr;
}

QHostAddress Request::address() const
{
    Q_D(const Request);
    return d->address;
}

quint16 Request::port() const
{
    Q_D(const Request);
    return d->port;
}

QUrl Request::uri() const
{
    Q_D(const Request);
    return d->uri;
}

QByteArray Request::base() const
{
    Q_D(const Request);
    return d->uri.toString(QUrl::RemoveUserInfo |
                           QUrl::RemovePath |
                           QUrl::RemoveQuery |
                           QUrl::RemoveFragment).toLocal8Bit();
}

QString Request::path() const
{
    Q_D(const Request);
    return d->path;
}

QStringList Request::args() const
{
    Q_D(const Request);
    return d->args;
}

QIODevice *Request::body() const
{
    Q_D(const Request);
    return d->body;
}

QMultiHash<QString, QString> Request::bodyParameters() const
{
    Q_D(const Request);
    return d->bodyParam;
}

QMultiHash<QString, QString> Request::bodyParam() const
{
    return bodyParameters();
}

QMultiHash<QString, QString> Request::queryParameters() const
{
    Q_D(const Request);
    return d->queryParam;
}

QMultiHash<QString, QString> Request::queryParam() const
{
    return queryParameters();
}

QMultiHash<QString, QString> Request::parameters() const
{
    Q_D(const Request);
    return d->param;
}

QMultiHash<QString, QString> Request::param() const
{
    return parameters();
}

QByteArray Request::contentEncoding() const
{
    Q_D(const Request);
    return d->headers.value("Content-Encoding");
}

QByteArray Request::contentType() const
{
    Q_D(const Request);
    return d->headers.value("Content-Type");
}

QNetworkCookie Request::cookie(const QByteArray &name) const
{
    Q_D(const Request);
    Q_FOREACH (const QNetworkCookie &cookie, d->cookies) {
        if (cookie.name() == name) {
            return cookie;
        }
    }
    return QNetworkCookie();
}

QList<QNetworkCookie> Request::cookies() const
{
    Q_D(const Request);
    return d->cookies;
}

QByteArray Request::header(const QByteArray &key) const
{
    Q_D(const Request);
    return d->headers.value(key);
}

QHash<QByteArray, QByteArray> Request::headers() const
{
    Q_D(const Request);
    return d->headers;
}

QByteArray Request::method() const
{
    Q_D(const Request);
    return d->method;
}

QByteArray Request::protocol() const
{
    Q_D(const Request);
    return d->protocol;
}

QByteArray Request::userAgent() const
{
    Q_D(const Request);
    return d->headers.value("User-Agent");
}

QByteArray Request::referer() const
{
    Q_D(const Request);
    return d->headers.value("Referer");
}

QByteArray Request::remoteUser() const
{
    Q_D(const Request);
    return d->remoteUser;
}

Engine *Request::engine() const
{
    Q_D(const Request);
    return d->engine;
}

void Request::setArgs(const QStringList &args)
{
    Q_D(Request);
    d->args = args;
}
