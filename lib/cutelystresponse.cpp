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

#include <QDebug>

CutelystResponse::CutelystResponse(QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystResponsePrivate)
{
}

CutelystResponse::~CutelystResponse()
{
    delete d_ptr;
}

quint16 CutelystResponse::status() const
{
    Q_D(const CutelystResponse);
    return d->status;
}

void CutelystResponse::setStatus(quint16 status)
{
    Q_D(CutelystResponse);
    d->status = status;
}

QString CutelystResponse::statusString() const
{
    Q_D(const CutelystResponse);
    switch (d->status) {
    case OK:
        return QLatin1String("OK");
    case MovedPermanently:
        return QLatin1String("Moved Permanently");
    case Found:
        return QLatin1String("Found");
    case NotModified:
        return QLatin1String("Not Modified");
    case TemporaryRedirect:
        return QLatin1String("Temporary Redirect");
    case BadRequest:
        return QLatin1String("Bad Request");
    case AuthorizationRequired:
        return QLatin1String("Authorization Required");
    case Forbidden:
        return QLatin1String("Forbidden");
    case NotFound:
        return QLatin1String("Not Found");
    case MethodNotAllowed:
        return QLatin1String("Method Not Allowed");
    case InternalServerError:
        return QLatin1String("Internal Server Error");
    }
    return QLatin1String("");
}

bool CutelystResponse::finalizedHeaders() const
{
    Q_D(const CutelystResponse);
    return d->finalizedHeaders;
}

QUrl CutelystResponse::redirect() const
{
    Q_D(const CutelystResponse);
    return d->redirect;
}

void CutelystResponse::setHeaderValue(const QString &key, const QString &value)
{
    Q_D(CutelystResponse);
    d->headers[key] = value;
}

bool CutelystResponse::hasBody() const
{
    Q_D(const CutelystResponse);
    return !d->body.isEmpty();
}

QByteArray CutelystResponse::body() const
{
    Q_D(const CutelystResponse);
    return d->body;
}

void CutelystResponse::setBody(const QByteArray &body)
{
    Q_D(CutelystResponse);
    d->body = body;
}

void CutelystResponse::setContentLength(quint64 length)
{
    Q_D(CutelystResponse);
    d->contentLength = length;
}

void CutelystResponse::setContentType(const QString &encoding)
{
    Q_D(CutelystResponse);
    d->contentType = encoding;
}

void CutelystResponse::redirect(const QString &url, quint16 status)
{
    Q_D(CutelystResponse);
    d->redirect = url;
    d->status = status;
}

QMap<QString, QString> CutelystResponse::headers() const
{
    Q_D(const CutelystResponse);

    QMap<QString, QString> ret = d->headers;
    ret.insert(QLatin1String("Content-Length"), QString::number(d->contentLength));
    ret.insert(QLatin1String("Content-Type"), d->contentType);
    // TODO use version macro here
    ret.insert(QLatin1String("X-Cutelyst"), QLatin1String("0.1"));

    return ret;
}

CutelystResponsePrivate::CutelystResponsePrivate() :
    finalizedHeaders(false),
    status(CutelystResponse::OK),
    contentLength(0)
{

}
