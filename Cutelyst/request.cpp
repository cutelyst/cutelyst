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

static QPair<QByteArray, QByteArray> nextField(const QByteArray &text, int &position);

Request::Request(RequestPrivate *prv) :
    d_ptr(prv)
{
}

Request::~Request()
{
    qDeleteAll(d_ptr->uploads);
    delete d_ptr;
}

QHostAddress Request::address() const
{
    Q_D(const Request);
    return d->remoteAddress;
}

inline QString remoteAddressLookup(const QString &remoteAddress)
{
    QHostInfo ptr = QHostInfo::fromName(remoteAddress);
    if (ptr.error() != QHostInfo::NoError) {
        qCWarning(CUTELYST_REQUEST) << "DNS lookup for the client hostname failed" << remoteAddress;
        return QString();
    }
    return ptr.hostName();
}

QString Request::hostname() const
{
    Q_D(const Request);
    static QString remoteHostName = remoteAddressLookup(d->remoteAddress.toString());
    return remoteHostName;
}

quint16 Request::port() const
{
    Q_D(const Request);
    return d->remotePort;
}

QUrl Request::uri() const
{
    Q_D(const Request);
    static QUrl url = d->parseUrl();
    return url;
}

QString Request::base() const
{
    Q_D(const Request);
    static QString base = d->parseBase();
    return base;
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

inline QString parseQueryKeywords(const QByteArray &query)
{
    QString keywords;
    if (query.indexOf('=') < 0) {
        keywords = QUrl::fromPercentEncoding(query);
    }
    return keywords;
}

QString Request::queryKeywords() const
{
    Q_D(const Request);
    static QString keywords = parseQueryKeywords(d->query);
    return keywords;
}

QVariantMap Request::queryParametersVariant() const
{
    return RequestPrivate::paramsMultiMapToVariantMap(queryParameters());
}

inline ParamsMultiMap parseUrlQuery(const QByteArray &query)
{
    ParamsMultiMap queryParam;
    if (query.indexOf('=') >= 0) {
        queryParam = RequestPrivate::parseUrlEncoded(query);
    }
    return queryParam;
}

ParamsMultiMap Request::queryParameters() const
{
    Q_D(const Request);
    static ParamsMultiMap queryParams = parseUrlQuery(d->query);
    return queryParams;
}

QVariantMap Request::parametersVariant() const
{
    return RequestPrivate::paramsMultiMapToVariantMap(parameters());
}

ParamsMultiMap Request::parameters() const
{
    static ParamsMultiMap params = ParamsMultiMap().unite(queryParameters()).unite(bodyParameters());
    return params;
}

QNetworkCookie Request::cookie(const QString &name) const
{
    Q_FOREACH (const QNetworkCookie &cookie, cookies()) {
        if (QString::fromLatin1(cookie.name()) == name) {
            return cookie;
        }
    }
    return QNetworkCookie();
}

inline QList<QNetworkCookie> parseCookies(const QByteArray &cookieString)
{
    QList<QNetworkCookie> ret;

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

    return ret;
}

QList<QNetworkCookie> Request::cookies() const
{
    Q_D(const Request);
    static QList<QNetworkCookie> cookies = parseCookies(d->headers.header(QStringLiteral("Cookie")).toLatin1());
    return cookies;
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

bool Request::isPost() const
{
    Q_D(const Request);
    return d->method == QStringLiteral("POST");
}

bool Request::isGet() const
{
    Q_D(const Request);
    return d->method == QStringLiteral("GET");
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
        auto it = args.constBegin();
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
    auto it = query.constBegin();
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

RequestPrivate::RequestPrivate(Engine *_engine,
                               const QString &_method,
                               const QString &_path,
                               const QByteArray &_query,
                               const QString &_protocol,
                               bool _isSecure,
                               const QString &_serverAddress,
                               const QString &_remoteAddress,
                               quint16 _remotePort,
                               const QString &_remoteUser,
                               const Headers &_headers,
                               quint64 _startOfRequest,
                               QIODevice *_body,
                               void *_requestPtr)
    : engine(_engine)
    , method(_method)
    , path(_path)
    , query(_query)
    , protocol(_protocol)
    , serverAddress(_serverAddress)
    , remoteAddress(_remoteAddress)
    , remoteUser(_remoteUser)
    , headers(_headers)
    , body(_body)
    , startOfRequest(_startOfRequest)
    , requestPtr(_requestPtr)
    , remotePort(_remotePort)
    , https(_isSecure)
{

}

QVariantMap RequestPrivate::paramsMultiMapToVariantMap(const ParamsMultiMap &params)
{
    QVariantMap ret;
    auto begin = params.constBegin();
    auto end = params.constEnd();
    while (begin != end) {
        --end;
        ret.insertMulti(ret.constBegin(), end.key(), end.value());
    }
    return ret;
}

QUrl RequestPrivate::parseUrl() const
{
    QUrl uri;

    // This is a hack just in case remote is not set
    if (serverAddress.isNull()) {
        uri.setHost(QHostInfo::localHostName());
    } else {
        uri.setAuthority(serverAddress);
    }

    uri.setScheme(https ? QStringLiteral("https") : QStringLiteral("http"));

    // if the path does not start with a slash it cleans the uri
    uri.setPath(QLatin1Char('/') % path);

    if (!query.isEmpty()) {
        uri.setQuery(QString::fromLatin1(query));
    }

    return uri;
}

QString RequestPrivate::parseBase() const
{
    QString base = https ? QStringLiteral("https://") : QStringLiteral("http://");

    // This is a hack just in case remote is not set
    if (serverAddress.isNull()) {
        base.append(QHostInfo::localHostName());
    } else {
        base.append(serverAddress);
    }

    // base always have a trailing slash
    base.append(QLatin1Char('/'));

    return base;
}
