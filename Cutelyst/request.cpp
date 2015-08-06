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

#include <QtCore/QStringBuilder>
#include <QtCore/QRegularExpression>
#include <QtCore/QJsonDocument>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkCookie>

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

        // if the path does not start with a slash it cleans the uri
        uri.setPath(QLatin1Char('/') % d->path);

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
        base.append(QLatin1Char('/'));

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

void Request::setMatch(const QString &match)
{
    Q_D(Request);
    d->match = match;
}

QStringList Request::arguments() const
{
    Q_D(const Request);
    return d->args;
}

void Request::setArguments(const QStringList &arguments)
{
    Q_D(Request);
    d->args = arguments;
}

QStringList Request::args() const
{
    Q_D(const Request);
    return d->args;
}

QStringList Request::captures() const
{
    Q_D(const Request);
    return d->captures;
}

void Request::setCaptures(const QStringList &captures)
{
    Q_D(Request);
    d->captures = captures;
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

QVariant Request::bodyData() const
{
    Q_D(const Request);
    if (!d->bodyParsed) {
        d->parseBody();
    }
    return d->bodyData;
}

QVariantMap Request::bodyParametersVariant() const
{
    return RequestPrivate::paramsMultiMapToVariantMap(bodyParameters());
}

ParamsMultiMap Request::bodyParameters() const
{
    Q_D(const Request);
    if (!d->bodyParsed) {
        d->parseBody();
    }
    return d->bodyParam;
}

QString Request::queryKeywords() const
{
    Q_D(const Request);
    if (!d->queryParamParsed) {
        d->parseUrlQuery();
    }
    return d->queryKeywords;
}

QVariantMap Request::queryParametersVariant() const
{
    return RequestPrivate::paramsMultiMapToVariantMap(queryParameters());
}

ParamsMultiMap Request::queryParameters() const
{
    Q_D(const Request);
    if (!d->queryParamParsed) {
        d->parseUrlQuery();
    }
    return d->queryParam;
}

QVariantMap Request::parametersVariant() const
{
    return RequestPrivate::paramsMultiMapToVariantMap(parameters());
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

ParamsMultiMap Request::mangleParams(const ParamsMultiMap &args, bool append) const
{
    ParamsMultiMap ret;
    if (append) {
        ret = args;
        ret.unite(queryParams());
    } else {
        ret = queryParams();
        ParamsMultiMap::ConstIterator it = args.constBegin();
        while (it != args.constEnd()) {
            ret.insert(it.key(), it.value());
            ++it;
        }
    }

    return ret;
}

QUrl Request::uriWith(const ParamsMultiMap &args, bool append) const
{
    QUrl ret = uri();
    QUrlQuery urlQuery;
    ParamsMultiMap query = mangleParams(args, append);
    ParamsMultiMap::ConstIterator it = query.constBegin();
    while (it != query.constEnd()) {
        urlQuery.addQueryItem(it.key(), it.value());
        ++it;
    }
    ret.setQuery(urlQuery);

    return ret;
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

void RequestPrivate::parseUrlQuery() const
{
    // Check for keywords (no = signs)
    if (query.indexOf('=') < 0) {
        queryKeywords = QUrl::fromPercentEncoding(query);
    } else {
        queryParam = parseUrlEncoded(query);
    }
    queryParamParsed = true;
}

void RequestPrivate::parseBody() const
{
    ParamsMultiMap params;
    QVariant data;

    const QString &contentType = headers.contentType();
    if (contentType == QLatin1String("application/x-www-form-urlencoded")) {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        qint64 posOrig = body->pos();
        body->seek(0);

        params = parseUrlEncoded(body->readLine());
        data = QVariant::fromValue(params);

        body->seek(posOrig);
    } else if (contentType == QLatin1String("multipart/form-data")) {
        Uploads uploadList = MultiPartFormDataParser::parse(body, headers.header(QStringLiteral("content_type")));
        for (int i = uploadList.size() - 1; i >= 0; --i) {
            Upload *upload = uploadList.at(i);
            uploads.insertMulti(upload->name(), upload);
        }
    } else if (contentType == QLatin1String("application/json")) {
        qint64 posOrig = body->pos();
        body->seek(0);

        data = QJsonDocument::fromJson(body->readAll());

        body->seek(posOrig);
    }

    // Asign it here so that we clean it in case no if matched
    bodyParam = params;
    bodyData = data;

    bodyParsed = true;
}

static inline bool isSlit(char c)
{
    return c == ';' || c == ',';
}

int findNextSplit(const QByteArray &text, int from, int length)
{
    while (from < length) {
        if (isSlit(text.at(from))) {
            return from;
        }
        ++from;
    }
    return -1;
}

static inline bool isLWS(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int nextNonWhitespace(const QByteArray &text, int from, int length)
{
    // RFC 2616 defines linear whitespace as:
    //  LWS = [CRLF] 1*( SP | HT )
    // We ignore the fact that CRLF must come as a pair at this point
    // It's an invalid HTTP header if that happens.
    while (from < length) {
        if (isLWS(text.at(from)))
            ++from;
        else
            return from;        // non-whitespace
    }

    // reached the end
    return text.length();
}

static QPair<QByteArray, QByteArray> nextField(const QByteArray &text, int &position)
{
    // format is one of:
    //    (1)  token
    //    (2)  token = token
    //    (3)  token = quoted-string
    const int length = text.length();
    position = nextNonWhitespace(text, position, length);

    int semiColonPosition = findNextSplit(text, position, length);
    if (semiColonPosition < 0)
        semiColonPosition = length; //no ';' means take everything to end of string

    int equalsPosition = text.indexOf('=', position);
    if (equalsPosition < 0 || equalsPosition > semiColonPosition) {
        return qMakePair(QByteArray(), QByteArray()); //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
    }

    QByteArray first = text.mid(position, equalsPosition - position).trimmed();
    QByteArray second;
    int secondLength = semiColonPosition - equalsPosition - 1;
    if (secondLength > 0)
        second = text.mid(equalsPosition + 1, secondLength).trimmed();

    position = semiColonPosition;
    return qMakePair(first, second);
}

void RequestPrivate::parseCookies() const
{
    QList<QNetworkCookie> ret;
    const QByteArray &cookieString = headers.header(QStringLiteral("Cookie")).toLatin1();
    int position = 0;
    const int length = cookieString.length();
    while (position < length) {
        QPair<QByteArray,QByteArray> field = nextField(cookieString, position);
        if (field.first.isEmpty()) {
            // parsing error
            break;
        }

        // Some foreign cookies are not in name=value format, so ignore them.
        if (field.second.isEmpty()) {
            ++position;
            continue;
        }
        ret.append(QNetworkCookie(field.first, field.second));
        ++position;

    }

    cookies = ret;
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

QVariantMap RequestPrivate::paramsMultiMapToVariantMap(const ParamsMultiMap &params)
{
    QVariantMap ret;
    ParamsMultiMap::const_iterator begin = params.constBegin();
    ParamsMultiMap::const_iterator end = params.constEnd();
    while (begin != end) {
        --end;
        ret.insertMulti(ret.constBegin(), end.key(), end.value());
    }
    return ret;
}

void RequestPrivate::reset()
{
    args = QStringList();
    captures = QStringList();
    urlParsed = false;
    baseParsed = false;
    cookiesParsed = false;
    queryParamParsed = false;
    queryKeywords.clear();
    queryParam.clear();
    bodyParsed = false;
    paramParsed = false;
    qDeleteAll(uploads);
    uploads.clear();
}
