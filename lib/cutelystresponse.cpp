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

bool CutelystResponse::finalizedHeaders() const
{
    Q_D(const CutelystResponse);
    return d->finalizedHeaders;
}

void CutelystResponse::addHeaderValue(const QString &key, const QByteArray &value)
{
    Q_D(CutelystResponse);
    d->headers.insertMulti(key, value);
}

bool CutelystResponse::hasBody() const
{
    Q_D(const CutelystResponse);
    return !d->body.isEmpty();
}

QByteArray &CutelystResponse::body()
{
    Q_D(CutelystResponse);
    return d->body;
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

QList<QNetworkCookie> CutelystResponse::cookies() const
{
    Q_D(const CutelystResponse);
    return d->cookies;
}

void CutelystResponse::addCookie(const QNetworkCookie &cookie)
{
    Q_D(CutelystResponse);
    d->cookies << cookie;
}

void CutelystResponse::setCookies(const QList<QNetworkCookie> &cookies)
{
    Q_D(CutelystResponse);
    d->cookies = cookies;
}

void CutelystResponse::redirect(const QString &url, quint16 status)
{
    Q_D(CutelystResponse);
    d->location = url;
    d->status = status;
}

QUrl CutelystResponse::location() const
{
    Q_D(const CutelystResponse);
    return d->location;
}

QMap<QString, QString> CutelystResponse::headers() const
{
    Q_D(const CutelystResponse);

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

CutelystResponsePrivate::CutelystResponsePrivate() :
    finalizedHeaders(false),
    status(CutelystResponse::OK),
    contentLength(0)
{

}
