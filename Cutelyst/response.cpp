/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "response_p.h"

#include "context.h"
#include "engine.h"
#include "common.h"

#include <QBuffer>

using namespace Cutelyst;

Response::Response(Context *c) : QObject(c)
  , d_ptr(new ResponsePrivate)
{
    d_ptr->context = c;
}

Response::~Response()
{
    delete d_ptr->body;
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

bool Response::hasBody() const
{
    Q_D(const Response);
    return d->body;
}

QByteArray &Response::body()
{
    Q_D(Response);

    QBuffer *buf = qobject_cast<QBuffer*>(d->body);
    if (!buf) {
        buf = new QBuffer;
        if (!buf->open(QIODevice::ReadWrite)) {
            qCCritical(CUTELYST_RESPONSE) << "Could not open QBuffer!";
        }
        d->body = buf;
    }
    return buf->buffer();
}

QIODevice *Response::bodyDevice()
{
    Q_D(Response);
    return d->body;
}

void Response::setBody(QIODevice *body)
{
    Q_D(Response);
    Q_ASSERT(body && body->isOpen() && body->isReadable());

    if (d->body && d->body != d->body) {
        delete d->body;
    }
    d->body = body;
}

QString Response::contentEncoding() const
{
    Q_D(const Response);
    return d->headers.contentEncoding();
}

void Cutelyst::Response::setContentEncoding(const QString &encoding)
{
    Q_D(Response);
    d->headers.setContentEncoding(encoding);
}

qint64 Response::contentLength() const
{
    Q_D(const Response);
    return d->headers.contentLength();
}

QString Response::contentType() const
{
    Q_D(const Response);
    return d->headers.contentType();
}

QString Response::contentTypeCharset() const
{
    Q_D(const Response);
    return d->headers.contentTypeCharset();
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

void Response::redirect(const QUrl &url, quint16 status)
{
    Q_D(Response);
    d->location = url;
    setStatus(status);
    if (url.isValid() && !d->body) {
        QBuffer *buf = new QBuffer;
        if (!buf->open(QIODevice::ReadWrite)) {
            qCCritical(CUTELYST_RESPONSE) << "Could not open QBuffer to write redirect!" << url << status;
            delete buf;
            return;
        }
        buf->write(QByteArrayLiteral("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0"
                                     "Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                                     "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                     "  <head>\n"
                                     "    <title>Moved</title>\n"
                                     "  </head>\n"
                                     "  <body>\n"
                                     "     <p>This item has moved <a href="));
        buf->write(url.toEncoded());
        buf->write(QByteArrayLiteral(">here</a>.</p>\n"
                                     "  </body>\n"
                                     "</html>\n"));
        d->body = buf;
        d->headers.setContentType(QStringLiteral("text/html; charset=utf-8"));
    }
}

void Response::redirect(const QString &url, quint16 status)
{
    redirect(QUrl(url), status);
}

QUrl Response::location() const
{
    Q_D(const Response);
    return d->location;
}

QString Response::header(const QString &field) const
{
    Q_D(const Response);
    return d->headers.header(field);
}

Headers &Response::headers()
{
    Q_D(Response);
    return d->headers;
}

void Response::write(const QByteArray &data)
{
    Q_D(Response);

    if (!d->finalizedHeaders) {

    }

    if (d->body) {
        d->body->write(data);
    } else {
        body().append(data);
    }
}
