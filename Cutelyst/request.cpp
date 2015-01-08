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

        // This is a hack just in case remote is not set
        if (d->serverAddress.isNull()) {
            uri.setHost(QHostInfo::localHostName());
        } else {
            uri.setAuthority(d->serverAddress);
        }
        uri.setScheme(d->https ? QStringLiteral("https") : QStringLiteral("http"));

        uri.setPath(d->path);
        if (!d->query.isEmpty()) {
            uri.setQuery(d->query);
        }

        d->url = uri;
        d->urlParsed = true;
    }
    return d->url;
}

QString Request::base() const
{
    Q_D(const Request);
    if (!d->baseParsed) {
        QString base = d->https ? QStringLiteral("https://") : QStringLiteral("http://");

        // This is a hack just in case remote is not set
        if (d->serverAddress.isNull()) {
            base.append(QHostInfo::localHostName());
        } else {
            base.append(d->serverAddress);
        }

        // base always have a trailing slash
        base.append(QChar('/'));

        d->base = base;
        d->baseParsed = true;
    }
    return d->base;
}

QString Request::path() const
{
    Q_D(const Request);
    return d->path;
}

QString Request::match() const
{
    Q_D(const Request);
    return d->match;
}

QStringList Request::arguments() const
{
    Q_D(const Request);
    return d->args;
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

QNetworkCookie Request::cookie(const QString &name) const
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

QString Request::method() const
{
    Q_D(const Request);
    return d->method;
}

QString Request::protocol() const
{
    Q_D(const Request);
    return d->protocol;
}

QString Request::remoteUser() const
{
    Q_D(const Request);
    return d->remoteUser;
}

QMap<QString, Cutelyst::Upload *> Request::uploads() const
{
    Q_D(const Request);
    if (!d->bodyParsed) {
        d->parseBody();
    }
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
    const QString &contentType = headers.contentType();
    if (contentType == QStringLiteral("application/x-www-form-urlencoded")) {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        qint64 posOrig = body->pos();
        body->seek(0);

        params = parseUrlEncoded(body->readLine());

        body->seek(posOrig);
    } else if (contentType.startsWith(QLatin1String("multipart/form-data"))) {
        MultiPartFormDataParser parser(contentType, body);
        Uploads uploadList = parser.parse();
        for (int i = uploadList.size() - 1; i >= 0; --i) {
            Upload *upload = uploadList.at(i);
            uploads.insertMulti(upload->name(), upload);
        }
    }

    // Asign it here so that we clean it in case no if matched
    bodyParam = params;

    bodyParsed = true;
}

void RequestPrivate::parseCookies() const
{
    QString cookiesHeader = headers.header(QStringLiteral("Cookie"));
    cookies = QNetworkCookie::parseCookies(cookiesHeader.replace(';', '\n').toLatin1());
    cookiesParsed = true;
}

ParamsMultiMap RequestPrivate::parseUrlEncoded(const QByteArray &line)
{
    ParamsMultiMap ret;
    const QList<QByteArray> &items = line.split('&');
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
    baseParsed = false;
    cookiesParsed = false;
    queryParamParsed = false;
    bodyParsed = false;
    paramParsed = false;
    qDeleteAll(uploads);
    uploads.clear();
}
