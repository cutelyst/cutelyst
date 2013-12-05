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

#include "cutelystresponse_p.h"

#include <QMimeDatabase>
#include <QDebug>

using namespace Cutelyst;

Response::Response(QObject *parent) :
    QObject(parent),
    d_ptr(new ResponsePrivate)
{
}

Response::~Response()
{
    delete d_ptr;
}

quint16 Response::status() const
{
    Q_D(const Response);
    return d->status;
}

void Response::setStatus(quint16 status)
{
    Q_D(Response);
    d->status = status;
}

bool Response::finalizedHeaders() const
{
    Q_D(const Response);
    return d->finalizedHeaders;
}

void Response::addHeaderValue(const QString &key, const QByteArray &value)
{
    Q_D(Response);
    d->headers.insertMulti(key, value);
}

bool Response::hasBody() const
{
    Q_D(const Response);
    return !d->body.isEmpty();
}

QByteArray &Response::body()
{
    Q_D(Response);
    return d->body;
}

void Response::setContentLength(quint64 length)
{
    Q_D(Response);
    d->contentLength = length;
}

QString Response::contentType() const
{
    Q_D(const Response);
    return d->contentType;
}

void Response::setContentType(const QString &encoding)
{
    Q_D(Response);
    d->contentType = encoding;
}

QList<QNetworkCookie> Response::cookies() const
{
    Q_D(const Response);
    return d->cookies;
}

void Response::addCookie(const QNetworkCookie &cookie)
{
    Q_D(Response);
    d->cookies << cookie;
}

void Response::setCookies(const QList<QNetworkCookie> &cookies)
{
    Q_D(Response);
    d->cookies = cookies;
}

void Response::redirect(const QString &url, quint16 status)
{
    Q_D(Response);
    d->location = url;
    d->status = status;
}

QUrl Response::location() const
{
    Q_D(const Response);
    return d->location;
}

QMap<QString, QString> Response::headers() const
{
    Q_D(const Response);

    QMultiMap<QString, QString> ret = d->headers;
    ret.insert(QLatin1String("Content-Length"), QString::number(d->contentLength));
    if (!d->contentType.isEmpty()) {
        ret.insert(QLatin1String("Content-Type"), d->contentType);
    } else if (!d->body.isEmpty()) {
        QMimeDatabase db;
        QMimeType mimeType = db.mimeTypeForData(d->body);
        if (mimeType.isValid()) {
            if (mimeType.name() == QLatin1String("text/html")) {
                ret.insert(QLatin1String("Content-Type"),
                           QLatin1String("text/html; charset=utf-8"));
            } else {
                ret.insert(QLatin1String("Content-Type"),
                           mimeType.name());
            }
        }
    }
    // TODO use version macro here
    ret.insert(QLatin1String("X-Cutelyst"), QLatin1String("0.1"));

    return ret;
}

ResponsePrivate::ResponsePrivate() :
    finalizedHeaders(false),
    status(Response::OK),
    contentLength(0)
{

}
