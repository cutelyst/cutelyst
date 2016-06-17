/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QtCore/QJsonDocument>
#include <QtNetwork/QHostInfo>

using namespace Cutelyst;

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

    const QHostInfo ptr = QHostInfo::fromName(d->remoteAddress.toString());
    if (ptr.error() != QHostInfo::NoError) {
        qCDebug(CUTELYST_REQUEST) << "DNS lookup for the client hostname failed" << d->remoteAddress;
        d->remoteHostname = QLatin1String("");
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
        uri.setPath(QLatin1Char('/') + d->path);

        if (!d->query.isEmpty()) {
            uri.setQuery(QString::fromLatin1(d->query));
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
        d->param = queryParameters();
        d->param.unite(bodyParameters());
        d->paramParsed = true;
    }
    return d->param;
}

QString Request::cookie(const QString &name) const
{
    Q_D(const Request);
    if (!d->cookiesParsed) {
        d->parseCookies();
    }

    return d->cookies.value(name);
}

QMap<QString, QString> Request::cookies() const
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

void RequestPrivate::parseUrlQuery() const
{
    // TODO move this to the asignment of query
    if (query.size()) {
        // Check for keywords (no = signs)
        if (query.indexOf('=') < 0) {
            queryKeywords = QUrl::fromPercentEncoding(query);
        } else {
            queryParam = parseUrlEncoded(query);
        }
    }
    queryParamParsed = true;
}

void RequestPrivate::parseBody() const
{
    const QString contentType = headers.contentType();
    if (contentType == QLatin1String("application/x-www-form-urlencoded")) {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        qint64 posOrig = body->pos();
        body->seek(0);

        bodyParam = parseUrlEncoded(body->readLine());
        bodyData = QVariant::fromValue(bodyParam);

        body->seek(posOrig);
    } else if (contentType == QLatin1String("multipart/form-data")) {
        Uploads uploadList = MultiPartFormDataParser::parse(body, headers.header(QStringLiteral("content_type")));
        auto it = uploadList.constEnd();
        while (it != uploadList.constBegin()) {
            --it;
            uploads.insertMulti((*it)->name(), *it);
        }
        bodyData = QVariant::fromValue(uploads);
    } else if (contentType == QLatin1String("application/json")) {
        qint64 posOrig = body->pos();
        body->seek(0);

        bodyData = QJsonDocument::fromJson(body->readAll());

        body->seek(posOrig);
    }

    bodyParsed = true;
}

static inline bool isSlit(QChar c)
{
    return c == QLatin1Char(';') || c == QLatin1Char(',');
}

int findNextSplit(const QString &text, int from, int length)
{
    while (from < length) {
        if (isSlit(text.at(from))) {
            return from;
        }
        ++from;
    }
    return -1;
}

static inline bool isLWS(QChar c)
{
    return c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\r') || c == QLatin1Char('\n');
}

static int nextNonWhitespace(const QString &text, int from, int length)
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

static QPair<QString, QString> nextField(const QString &text, int &position)
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

    int equalsPosition = text.indexOf(QLatin1Char('='), position);
    if (equalsPosition < 0 || equalsPosition > semiColonPosition) {
        return qMakePair(QString(), QString()); //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
    }

    QString first = text.mid(position, equalsPosition - position).trimmed();
    QString second;
    int secondLength = semiColonPosition - equalsPosition - 1;
    if (secondLength > 0)
        second = text.mid(equalsPosition + 1, secondLength).trimmed();

    position = semiColonPosition;
    return qMakePair(first, second);
}

void RequestPrivate::parseCookies() const
{
    QList<QPair<QString, QString> > ret;
    const QString cookieString = headers.header(QStringLiteral("Cookie"));
    int position = 0;
    const int length = cookieString.length();
    while (position < length) {
        QPair<QString,QString> field = nextField(cookieString, position);
        if (field.first.isEmpty()) {
            // parsing error
            break;
        }

        // Some foreign cookies are not in name=value format, so ignore them.
        if (field.second.isEmpty()) {
            ++position;
            continue;
        }
        ret.append(field);
        ++position;
    }

    auto it = ret.constEnd();
    while (it != ret.constBegin()) {
        --it;
        cookies.insertMulti(it->first, it->second);
    }

    cookiesParsed = true;
}

ParamsMultiMap RequestPrivate::parseUrlEncoded(const QByteArray &line)
{
    ParamsMultiMap ret;
    const QList<QByteArray> items = line.split('&');
    for (int i = items.size() - 1; i >= 0; --i) {
        const QByteArray parameter = items.at(i);
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

#include "moc_request.cpp"
