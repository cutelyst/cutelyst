/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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
#include <QHostInfo>

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
    return d->remoteAddress;
}

quint16 Request::port() const
{
    Q_D(const Request);
    return d->remotePort;
}

QUrl Request::uri() const
{
    Q_D(const Request);
    if (!d->urlParsed) {
        QUrl uri;
        if (d->serverAddress.isNull()) {
            // This is a hack just in case remote is not set
            uri.setHost(QHostInfo::localHostName());
        } else {
            uri.setHost(d->serverAddress);
            // Append the server port if different from default one
            if ((d->https && d->serverPort != 443) || (!d->https && d->serverPort != 80)) {
                uri.setPort(d->serverPort);
            }
        }
        uri.setScheme(d->https ? QStringLiteral("https") : QStringLiteral("http"));
        uri.setPath(d->path);
        uri.setQuery(d->query);

        d->url = uri;
        d->urlParsed = true;
    }
    return d->url;
}

QByteArray Request::base() const
{
    return uri().toString(QUrl::RemoveUserInfo |
                          QUrl::RemovePath |
                          QUrl::RemoveQuery |
                          QUrl::RemoveFragment).toLatin1();
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

QMultiHash<QString, QString> Request::queryParameters() const
{
    Q_D(const Request);
    if (!d->queryParamParsed) {
        d->parseUrlQuery();
    }
    return d->queryParam;
}

QMultiHash<QString, QString> Request::parameters() const
{
    Q_D(const Request);
    if (!d->paramParsed) {
        d->param = queryParameters() + bodyParameters();
        d->paramParsed = true;
    }
    return d->param;
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

void *Request::engineData()
{
    Q_D(Request);
    return d->requestPtr;
}

void Request::setArgs(const QStringList &args)
{
    Q_D(Request);
    d->args = args;
}

void RequestPrivate::parseUrlQuery() const
{
    QUrlQuery urlQuery(query);
    QMultiHash<QString, QString> params;
    Q_FOREACH (const StringPair &queryItem, urlQuery.queryItems()) {
        params.insertMulti(queryItem.first, queryItem.second);
    }
    queryParam = params;
    queryParamParsed = true;
}

void RequestPrivate::parseBody() const
{
    QMultiHash<QString, QString> params;
    const QByteArray &contentType = headers.contentType();
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
                params.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                   QUrl::fromPercentEncoding(value));
            } else {
                params.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                                   QString());
            }
        }
        body->seek(posOrig);
        bodyParam = params;
    } else if (contentType.startsWith("multipart/form-data")) {
        MultiPartFormDataParser parser(contentType, body);
        uploads = parser.parse();
    }

    bodyParsed = true;
}

void RequestPrivate::parseCookies() const
{
    QByteArray cookiesHeader = headers.header("Cookie");
    cookies = QNetworkCookie::parseCookies(cookiesHeader.replace(';', '\n'));
    cookiesParsed = true;
}

void RequestPrivate::reset()
{
    args = QStringList();
    urlParsed = false;
    cookiesParsed = false;
    queryParamParsed = false;
    bodyParsed = false;
    paramParsed = false;
    qDeleteAll(uploads);
}
