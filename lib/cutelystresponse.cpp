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

CutelystResponse::CutelystResponse(QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystResponsePrivate)
{
}

CutelystResponse::~CutelystResponse()
{
    delete d_ptr;
}

int CutelystResponse::status() const
{
    return 0;
}

bool CutelystResponse::finalizedHeaders() const
{
    Q_D(const CutelystResponse);
    return d->finalizedHeaders;
}

QString CutelystResponse::redirect() const
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

void CutelystResponse::setRedirect(const QString &url, quint16 status)
{
    Q_D(CutelystResponse);
    d->redirect = url;
    d->status = status;
}

CutelystResponsePrivate::CutelystResponsePrivate() :
    finalizedHeaders(false),
    status(0),
    contentLength(0)
{

}
