/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "request_p.h"
#include "engine.h"
#include "enginerequest.h"
#include "common.h"
#include "multipartformdataparser.h"
#include "utils.h"

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
    return d->engineRequest->remoteAddress;
}

QString Request::addressString() const
{
    Q_D(const Request);

    bool ok;
    quint32 data = d->engineRequest->remoteAddress.toIPv4Address(&ok);
    if (ok) {
        return QHostAddress(data).toString();
    } else {
        return d->engineRequest->remoteAddress.toString();
    }
}

QString Request::hostname() const
{
    Q_D(const Request);
    QString ret;

    // We have the client hostname
    if (!d->remoteHostname.isEmpty()) {
        ret = d->remoteHostname;
        return ret;
    } else {
        // We tried to get the client hostname but failed
        if (!d->remoteHostname.isEmpty()) {
            return ret;
        }
    }

    const QHostInfo ptr = QHostInfo::fromName(d->engineRequest->remoteAddress.toString());
    if (ptr.error() != QHostInfo::NoError) {
        qCDebug(CUTELYST_REQUEST) << "DNS lookup for the client hostname failed" << d->engineRequest->remoteAddress;
        d->remoteHostname = QStringLiteral("");
        return ret;
    }

    d->remoteHostname = ptr.hostName();
    ret = d->remoteHostname;
    return ret;
}

quint16 Request::port() const
{
    Q_D(const Request);
    return d->engineRequest->remotePort;
}

QUrl Request::uri() const
{
    Q_D(const Request);

    QUrl uri = d->url;
    if (!(d->parserStatus & RequestPrivate::UrlParsed)) {
        // This is a hack just in case remote is not set
        if (d->engineRequest->serverAddress.isEmpty()) {
            uri.setHost(QHostInfo::localHostName());
        } else {
            uri.setAuthority(d->engineRequest->serverAddress);
        }

        uri.setScheme(d->engineRequest->isSecure ? QStringLiteral("https") : QStringLiteral("http"));

        // if the path does not start with a slash it cleans the uri
        uri.setPath(QLatin1Char('/') + d->engineRequest->path);

        if (!d->engineRequest->query.isEmpty()) {
            uri.setQuery(QString::fromLatin1(d->engineRequest->query));
        }

        d->url = uri;
        d->parserStatus |= RequestPrivate::UrlParsed;
    }
    return uri;
}

QString Request::base() const
{
    Q_D(const Request);
    QString base = d->base;
    if (!(d->parserStatus & RequestPrivate::BaseParsed)) {
        base = d->engineRequest->isSecure ? QStringLiteral("https://") : QStringLiteral("http://");

        // This is a hack just in case remote is not set
        if (d->engineRequest->serverAddress.isEmpty()) {
            base.append(QHostInfo::localHostName());
        } else {
            base.append(d->engineRequest->serverAddress);
        }

        // base always have a trailing slash
        base.append(QLatin1Char('/'));

        d->base = base;
        d->parserStatus |= RequestPrivate::BaseParsed;
    }
    return base;
}

QString Request::path() const
{
    Q_D(const Request);
    return d->engineRequest->path;
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
    return d->engineRequest->isSecure;
}

QIODevice *Request::body() const
{
    Q_D(const Request);
    return d->engineRequest->body;
}

QVariant Request::bodyData() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
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
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
        d->parseBody();
    }
    return d->bodyParam;
}

QString Request::queryKeywords() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::QueryParsed)) {
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
    if (!(d->parserStatus & RequestPrivate::QueryParsed)) {
        d->parseUrlQuery();
    }
    return d->queryParam;
}

QString Request::cookie(const QString &name) const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::CookiesParsed)) {
        d->parseCookies();
    }

    return d->cookies.value(name);
}

QMap<QString, QString> Request::cookies() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::CookiesParsed)) {
        d->parseCookies();
    }
    return d->cookies;
}

Headers Request::headers() const
{
    Q_D(const Request);
    return d->engineRequest->headers;
}

QString Request::method() const
{
    Q_D(const Request);
    return d->engineRequest->method;
}

bool Request::isPost() const
{
    Q_D(const Request);
    return d->engineRequest->method == QStringLiteral("POST");
}

bool Request::isGet() const
{
    Q_D(const Request);
    return d->engineRequest->method == QStringLiteral("GET");
}

QString Request::protocol() const
{
    Q_D(const Request);
    return d->engineRequest->protocol;
}

QString Request::remoteUser() const
{
    Q_D(const Request);
    return d->engineRequest->remoteUser;
}

QVector<Upload *> Request::uploads() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
        d->parseBody();
    }
    return d->uploads;
}

QMap<QString, Cutelyst::Upload *> Request::uploadsMap() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
        d->parseBody();
    }
    return d->uploadsMap;
}

Uploads Request::uploads(const QString &name) const
{
    Uploads ret;
    const auto map = uploadsMap();
    const auto range = map.equal_range(name);
    for (auto i = range.first; i != range.second; ++i) {
        ret.push_back(*i);
    }
    return ret;
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
        const auto end = args.constEnd();
        while (it != end) {
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
    const auto end = query.constEnd();
    while (it != end) {
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

void RequestPrivate::parseUrlQuery() const
{
    // TODO move this to the asignment of query
    if (engineRequest->query.size()) {
        // Check for keywords (no = signs)
        if (engineRequest->query.indexOf('=') < 0) {
            QByteArray aux = engineRequest->query;
            queryKeywords = Utils::decodePercentEncoding(&aux);
        } else {
            queryParam = parseUrlEncoded(engineRequest->query);
        }
    }
    parserStatus |= RequestPrivate::QueryParsed;
}

void RequestPrivate::parseBody() const
{
    if (!engineRequest->body) {
        parserStatus |= RequestPrivate::BodyParsed;
        return;
    }

    bool sequencial = engineRequest->body->isSequential();
    qint64 posOrig = engineRequest->body->pos();
    if (sequencial && posOrig) {
        qCWarning(CUTELYST_REQUEST) << "Can not parse sequential post body out of beginning";
        parserStatus |= RequestPrivate::BodyParsed;
        return;
    }

    const QString contentType = engineRequest->headers.contentType();
    if (contentType == QLatin1String("application/x-www-form-urlencoded")) {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        if (posOrig) {
            engineRequest->body->seek(0);
        }

        bodyParam = parseUrlEncoded(engineRequest->body->readLine());
        bodyData = QVariant::fromValue(bodyParam);
    } else if (contentType == QLatin1String("multipart/form-data")) {
        if (posOrig) {
            engineRequest->body->seek(0);
        }

        uploads = MultiPartFormDataParser::parse(engineRequest->body, engineRequest->headers.header(QStringLiteral("CONTENT_TYPE")));
        auto it = uploads.crbegin();
        while (it != uploads.crend()) {
            Upload *upload = *it;
            uploadsMap.insertMulti(upload->name(), upload);
            ++it;
        }
        bodyData = QVariant::fromValue(uploadsMap);
    } else if (contentType == QLatin1String("application/json")) {
        if (posOrig) {
            engineRequest->body->seek(0);
        }

        bodyData = QJsonDocument::fromJson(engineRequest->body->readAll());
    }

    if (!sequencial) {
        engineRequest->body->seek(posOrig);
    }

    parserStatus |= RequestPrivate::BodyParsed;
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

static std::pair<QString, QString> nextField(const QString &text, int &position)
{
    std::pair<QString, QString> ret;
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
        return ret; //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
    }

    ret.first = text.mid(position, equalsPosition - position).trimmed();
    int secondLength = semiColonPosition - equalsPosition - 1;
    if (secondLength > 0) {
        ret.second = text.mid(equalsPosition + 1, secondLength).trimmed();
    }

    position = semiColonPosition;
    return ret;
}

void RequestPrivate::parseCookies() const
{
    std::vector<std::pair<QString, QString> > ret;
    const QString cookieString = engineRequest->headers.header(QStringLiteral("COOKIE"));
    int position = 0;
    const int length = cookieString.length();
    while (position < length) {
        const auto field = nextField(cookieString, position);
        if (field.first.isEmpty()) {
            // parsing error
            break;
        }

        // Some foreign cookies are not in name=value format, so ignore them.
        if (field.second.isEmpty()) {
            ++position;
            continue;
        }
        ret.push_back(field);
        ++position;
    }

    auto i = ret.crbegin();
    const auto end = ret.crend();
    while (i != end) {
        cookies.insertMulti(i->first, i->second);
        ++i;
    }

    parserStatus |= RequestPrivate::CookiesParsed;
}

ParamsMultiMap RequestPrivate::parseUrlEncoded(const QByteArray &line)
{
    ParamsMultiMap ret;
    int from = line.length();
    int pos = from;

    while (pos > 0) {
        from = pos - 1;
        pos = line.lastIndexOf('&', from);

        int len = from - pos;
        if (len == 0) {
            // Skip empty strings
            --from;
            continue;
        }

        QByteArray data = line.mid(pos + 1, len);

        int equal = data.indexOf('=');
        if (equal != -1) {
            QByteArray value = data.mid(equal + 1);
            if (value.length()) {
                QByteArray key = data.mid(0, equal);
                ret.insertMulti(Utils::decodePercentEncoding(&key),
                                Utils::decodePercentEncoding(&value));
            }
        } else {
            ret.insertMulti(Utils::decodePercentEncoding(&data),
                            QString());
        }
    }

    return ret;
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
