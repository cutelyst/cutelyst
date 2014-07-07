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
#include "multipartformdataparser.h"

#include <QStringBuilder>
#include <QRegularExpression>
#include <QTime>

using namespace Cutelyst;

Request::Request(RequestPrivate *prv) :
    d_ptr(prv)
{

}

Request::~Request()
{
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
    if (!d->bodyParsed) {
        d->parseBody();
    }
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
    if (!d->bodyParsed) {
        d->parseBody();
    }
    return d->param;
}

QMultiHash<QString, QString> Request::param() const
{
    return parameters();
}

QByteArray Request::contentEncoding() const
{
    Q_D(const Request);
    return d->headers.header("Content-Encoding");
}

QByteArray Request::contentType() const
{
    Q_D(const Request);
    return d->headers.header("Content-Type");
}

QNetworkCookie Request::cookie(const QByteArray &name) const
{
    Q_D(const Request);
    if (!d->cookiesParsed) {
        d->parseCookies();
    }

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
    if (!d->cookiesParsed) {
        d->parseCookies();
    }
    return d->cookies;
}

QByteArray Request::header(const QByteArray &key) const
{
    Q_D(const Request);
    return d->headers.header(key);
}

Headers Request::headers() const
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
    return d->headers.header("User-Agent");
}

QByteArray Request::referer() const
{
    Q_D(const Request);
    return d->headers.header("Referer");
}

QByteArray Request::remoteUser() const
{
    Q_D(const Request);
    return d->remoteUser;
}

Uploads Request::uploads() const
{
    Q_D(const Request);
    return d->uploads;
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

void RequestPrivate::parseBody() const
{
    const QByteArray &contentType = headers.header("Content-Type");
    if (contentType == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        qint64 posOrig = body->pos();
        body->seek(0);
        QByteArray bodyArray = body->readLine();
        Q_FOREACH (const QByteArray &parameter, bodyArray.split('&')) {
            if (parameter.isEmpty()) {
                continue;
            }

            QList<QByteArray> parts = parameter.split('=');
            if (parts.size() == 2) {
                QByteArray value = parts.at(1);
                value.replace('+', ' ');
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                      QUrl::fromPercentEncoding(value));
            } else {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                                      QString());
            }
        }
        body->seek(posOrig);
        param = queryParam + bodyParam;
    } else if (contentType.startsWith("multipart/form-data")) {
        MultiPartFormDataParser parser(contentType, body);
        uploads = parser.parse();
    } else {
        param = queryParam;
    }

    bodyParsed = true;
}

void RequestPrivate::parseCookies() const
{
    QByteArray cookiesHeader = headers.header("Cookie");
    cookies = QNetworkCookie::parseCookies(cookiesHeader.replace(';', '\n'));
    cookiesParsed = true;
}
