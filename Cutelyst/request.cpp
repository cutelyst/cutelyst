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
#include "common.h"
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

QString Request::hostname() const
{
    Q_D(const Request);

    // We have the client hostname
    if (!d->remoteHostname.isEmpty()) {
        return d->remoteHostname;
    } else {
        // We tried to get the client hostname but failed
        if (!d->remoteHostname.isNull()) {
            return QString();
        }
    }

    QHostInfo ptr = QHostInfo::fromName(d->remoteAddress.toString());
    if (ptr.error() != QHostInfo::NoError) {
        qCDebug(CUTELYST_REQUEST) << "DNS lookup for the client hostname failed" << d->remoteAddress;
        d->remoteHostname = "";
        return QString();
    }

    d->remoteHostname = ptr.hostName();
    return d->remoteHostname;
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

QUrl Request::base() const
{
    return uri().adjusted(QUrl::RemoveUserInfo |
                          QUrl::RemovePath |
                          QUrl::RemoveQuery |
                          QUrl::RemoveFragment);
}

QByteArray Request::path() const
{
    Q_D(const Request);
    return d->path;
}

QByteArray Request::match() const
{
    Q_D(const Request);
    return d->match;
}

QStringList Request::args() const
{
    Q_D(const Request);
    return d->args;
}

bool Request::secure() const
{
    Q_D(const Request);
    return d->https;
}

QIODevice *Request::body() const
{
    Q_D(const Request);
    return d->body;
}

ParamsMultiMap Request::bodyParameters() const
{
    Q_D(const Request);
    if (!d->bodyParsed) {
        d->parseBody();
    }
    return d->bodyParam;
}

ParamsMultiMap Request::queryParameters() const
{
    Q_D(const Request);
    if (!d->queryParamParsed) {
        d->parseUrlQuery();
    }
    return d->queryParam;
}

ParamsMultiMap Request::parameters() const
{
    Q_D(const Request);
    if (!d->paramParsed) {
        d->param = queryParameters().unite(bodyParameters());
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
    queryParam = parseUrlEncoded(query);
    queryParamParsed = true;
}

void RequestPrivate::parseBody() const
{
    ParamsMultiMap params;
    const QByteArray &contentType = headers.contentType();
    if (contentType == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        qint64 posOrig = body->pos();
        body->seek(0);

        params = parseUrlEncoded(body->readLine());

        body->seek(posOrig);
    } else if (contentType.startsWith("multipart/form-data")) {
        MultiPartFormDataParser parser(contentType, body);
        uploads = parser.parse();
    }

    // Asign it here so that we clean it in case no if matched
    bodyParam = params;

    bodyParsed = true;
}

void RequestPrivate::parseCookies() const
{
    QByteArray cookiesHeader = headers.header("Cookie");
    cookies = QNetworkCookie::parseCookies(cookiesHeader.replace(';', '\n'));
    cookiesParsed = true;
}

ParamsMultiMap RequestPrivate::parseUrlEncoded(const QByteArray &line)
{
    ParamsMultiMap ret;
    QList<QByteArray> items = line.split('&');
    for (int i = items.size() - 1; i >= 0; --i) {
        const QByteArray &parameter = items.at(i);
        if (parameter.isEmpty()) {
            continue;
        }

        const QList<QByteArray> &parts = parameter.split('=');
        if (parts.size() == 2) {
            QByteArray value = parts.at(1);
            if (value.length()) {
                ret.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                QUrl::fromPercentEncoding(value.replace('+', ' ')));
                continue;
            }
        }
        ret.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                        QString());

    }
    return ret;
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
