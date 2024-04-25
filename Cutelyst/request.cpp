/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "common.h"
#include "engine.h"
#include "enginerequest.h"
#include "multipartformdataparser.h"
#include "request_p.h"
#include "utils.h"

#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Cutelyst;

Request::Request(Cutelyst::EngineRequest *engineRequest)
    : d_ptr(new RequestPrivate)
{
    d_ptr->engineRequest = engineRequest;
    d_ptr->body          = engineRequest->body;
}

Request::~Request()
{
    qDeleteAll(d_ptr->uploads);
    delete d_ptr->body;
    delete d_ptr;
}

QHostAddress Request::address() const noexcept
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
    }

    const QHostInfo ptr = QHostInfo::fromName(d->engineRequest->remoteAddress.toString());
    if (ptr.error() != QHostInfo::NoError) {
        qCDebug(CUTELYST_REQUEST) << "DNS lookup for the client hostname failed"
                                  << d->engineRequest->remoteAddress;
        return ret;
    }

    d->remoteHostname = ptr.hostName();
    ret               = d->remoteHostname;
    return ret;
}

quint16 Request::port() const noexcept
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
            uri.setAuthority(QString::fromLatin1(d->engineRequest->serverAddress));
        }

        uri.setScheme(d->engineRequest->isSecure ? QStringLiteral("https")
                                                 : QStringLiteral("http"));

        // if the path does not start with a slash it cleans the uri
        // TODO check if engines will always set a slash
        uri.setPath(d->engineRequest->path);

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
            base.append(QString::fromLatin1(d->engineRequest->serverAddress));
        }

        d->base = base;
        d->parserStatus |= RequestPrivate::BaseParsed;
    }
    return base;
}

QString Request::path() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->path;
}

QString Request::match() const noexcept
{
    Q_D(const Request);
    return d->match;
}

void Request::setMatch(const QString &match)
{
    Q_D(Request);
    d->match = match;
}

QStringList Request::arguments() const noexcept
{
    Q_D(const Request);
    return d->args;
}

void Request::setArguments(const QStringList &arguments)
{
    Q_D(Request);
    d->args = arguments;
}

QStringList Request::captures() const noexcept
{
    Q_D(const Request);
    return d->captures;
}

void Request::setCaptures(const QStringList &captures)
{
    Q_D(Request);
    d->captures = captures;
}

bool Request::secure() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->isSecure;
}

QIODevice *Request::body() const noexcept
{
    Q_D(const Request);
    return d->body;
}

QVariant Request::bodyData() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
        d->parseBody();
    }
    return d->bodyData;
}

QCborValue Request::bodyCbor() const
{
    return bodyData().value<QCborValue>();
}

QJsonDocument Request::bodyJsonDocument() const
{
    return bodyData().toJsonDocument();
}

QJsonObject Request::bodyJsonObject() const
{
    return bodyData().toJsonDocument().object();
}

QJsonArray Request::bodyJsonArray() const
{
    return bodyData().toJsonDocument().array();
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

QStringList Request::bodyParameters(const QString &key) const
{
    QStringList ret;

    const ParamsMultiMap query = bodyParameters();
    auto it                    = query.constFind(key);
    while (it != query.constEnd() && it.key() == key) {
        ret.prepend(it.value());
        ++it;
    }
    return ret;
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

QStringList Request::queryParameters(const QString &key) const
{
    QStringList ret;

    const ParamsMultiMap query = queryParameters();
    auto it                    = query.constFind(key);
    while (it != query.constEnd() && it.key() == key) {
        ret.prepend(it.value());
        ++it;
    }
    return ret;
}

QByteArray Request::cookie(QByteArrayView name) const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::CookiesParsed)) {
        d->parseCookies();
    }

    return d->cookies.value(name).value;
}

QByteArrayList Request::cookies(QByteArrayView name) const
{
    QByteArrayList ret;
    Q_D(const Request);

    if (!(d->parserStatus & RequestPrivate::CookiesParsed)) {
        d->parseCookies();
    }

    for (auto it = d->cookies.constFind(name); it != d->cookies.constEnd() && it->name == name;
         ++it) {
        ret.prepend(it->value);
    }
    return ret;
}

QMultiMap<QByteArrayView, Request::Cookie> Request::cookies() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::CookiesParsed)) {
        d->parseCookies();
    }
    return d->cookies;
}

Headers Request::headers() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->headers;
}

QByteArray Request::method() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method;
}

bool Request::isPost() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("POST") == 0;
}

bool Request::isGet() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("GET") == 0;
}

bool Request::isHead() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("HEAD") == 0;
}

bool Request::isPut() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("PUT") == 0;
}

bool Request::isPatch() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("PATCH") == 0;
}

bool Request::isDelete() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->method.compare("DELETE") == 0;
}

QByteArray Request::protocol() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->protocol;
}

bool Request::xhr() const noexcept
{
    Q_D(const Request);
    return d->engineRequest->headers.header("X-Requested-With").compare("XMLHttpRequest") == 0;
}

QString Request::remoteUser() const noexcept
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

QMultiMap<QStringView, Cutelyst::Upload *> Request::uploadsMap() const
{
    Q_D(const Request);
    if (!(d->parserStatus & RequestPrivate::BodyParsed)) {
        d->parseBody();
    }
    return d->uploadsMap;
}

Uploads Request::uploads(QStringView name) const
{
    Uploads ret;
    const auto map   = uploadsMap();
    const auto range = map.equal_range(name);
    for (auto i = range.first; i != range.second; ++i) {
        ret.push_back(*i);
    }
    return ret;
}

ParamsMultiMap Request::mangleParams(const ParamsMultiMap &args, bool append) const
{
    ParamsMultiMap ret = queryParams();
    if (append) {
        ret.unite(args);
    } else {
        auto it = args.constEnd();
        while (it != args.constBegin()) {
            --it;
            ret.replace(it.key(), it.value());
        }
    }

    return ret;
}

QUrl Request::uriWith(const ParamsMultiMap &args, bool append) const
{
    QUrl ret = uri();
    QUrlQuery urlQuery;
    const ParamsMultiMap query = mangleParams(args, append);
    auto it                    = query.constEnd();
    while (it != query.constBegin()) {
        --it;
        urlQuery.addQueryItem(it.key(), it.value());
    }
    ret.setQuery(urlQuery);

    return ret;
}

Engine *Request::engine() const noexcept
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
            queryKeywords  = Utils::decodePercentEncoding(&aux);
        } else {
            if (parserStatus & RequestPrivate::UrlParsed) {
                queryParam = Utils::decodePercentEncoding(engineRequest->query.data(),
                                                          engineRequest->query.size());
            } else {
                QByteArray aux = engineRequest->query;
                // We can't manipulate query directly
                queryParam = Utils::decodePercentEncoding(aux.data(), aux.size());
            }
        }
    }
    parserStatus |= RequestPrivate::QueryParsed;
}

void RequestPrivate::parseBody() const
{
    if (!body) {
        parserStatus |= RequestPrivate::BodyParsed;
        return;
    }

    bool sequencial = body->isSequential();
    qint64 posOrig  = body->pos();
    if (sequencial && posOrig) {
        qCWarning(CUTELYST_REQUEST) << "Can not parse sequential post body out of beginning";
        parserStatus |= RequestPrivate::BodyParsed;
        return;
    }

    const QByteArray contentType = engineRequest->headers.header("Content-Type");
    if (contentType.startsWith("application/x-www-form-urlencoded")) {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        if (posOrig) {
            body->seek(0);
        }

        QByteArray line = body->readAll();
        bodyParam       = Utils::decodePercentEncoding(line.data(), line.size());
        bodyData        = QVariant::fromValue(bodyParam);
    } else if (contentType.startsWith("multipart/form-data")) {
        if (posOrig) {
            body->seek(0);
        }

        const Uploads ups = MultiPartFormDataParser::parse(body, contentType);
        for (Upload *upload : ups) {
            if (upload->filename().isEmpty() &&
                upload->headers().header("Content-Type"_qba).isEmpty()) {
                bodyParam.insert(upload->name(), QString::fromUtf8(upload->readAll()));
                upload->seek(0);
            }
            uploadsMap.insert(upload->name(), upload);
        }
        uploads = ups;
        //        bodyData = QVariant::fromValue(uploadsMap);
    } else if (contentType.startsWith("application/cbor")) {
        if (posOrig) {
            body->seek(0);
        }

        bodyData = QVariant::fromValue(QCborValue::fromCbor(body->readAll()));
    } else if (contentType.startsWith("application/json")) {
        if (posOrig) {
            body->seek(0);
        }

        bodyData = QJsonDocument::fromJson(body->readAll());
    }

    if (!sequencial) {
        body->seek(posOrig);
    }

    parserStatus |= RequestPrivate::BodyParsed;
}

static inline bool isSlit(char c)
{
    return c == ';' || c == ',';
}

int findNextSplit(QByteArrayView text, int from, int length)
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

static int nextNonWhitespace(QByteArrayView text, int from, int length)
{
    // RFC 2616 defines linear whitespace as:
    //  LWS = [CRLF] 1*( SP | HT )
    // We ignore the fact that CRLF must come as a pair at this point
    // It's an invalid HTTP header if that happens.
    while (from < length) {
        if (isLWS(text.at(from)))
            ++from;
        else
            return from; // non-whitespace
    }

    // reached the end
    return text.length();
}

static Request::Cookie nextField(QByteArrayView text, int &position)
{
    Request::Cookie cookie;
    // format is one of:
    //    (1)  token
    //    (2)  token = token
    //    (3)  token = quoted-string
    const int length = text.length();
    position         = nextNonWhitespace(text, position, length);

    int semiColonPosition = findNextSplit(text, position, length);
    if (semiColonPosition < 0)
        semiColonPosition = length; // no ';' means take everything to end of string

    int equalsPosition = text.indexOf('=', position);
    if (equalsPosition < 0 || equalsPosition > semiColonPosition) {
        return cookie; //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
    }

    // TODO Qt 6.3
    //    ret.first = text.sliced(position, equalsPosition - position).trimmed().toByteArray();
    cookie.name      = text.sliced(position, equalsPosition - position).toByteArray().trimmed();
    int secondLength = semiColonPosition - equalsPosition - 1;
    if (secondLength > 0) {
        // TODO Qt 6.3
        //        ret.second = text.sliced(equalsPosition + 1,
        //        secondLength).trimmed().toByteArray();
        cookie.value = text.sliced(equalsPosition + 1, secondLength).toByteArray().trimmed();
    }

    position = semiColonPosition;
    return cookie;
}

void RequestPrivate::parseCookies() const
{
    const QByteArray cookieString = engineRequest->headers.header("Cookie"_qba);
    int position                  = 0;
    const int length              = cookieString.length();
    while (position < length) {
        const auto cookie = nextField(cookieString, position);
        if (cookie.name.isEmpty()) {
            // parsing error
            break;
        }

        // Some foreign cookies are not in name=value format, so ignore them.
        if (cookie.value.isEmpty()) {
            ++position;
            continue;
        }
        cookies.insert(cookie.name, cookie);
        ++position;
    }

    parserStatus |= RequestPrivate::CookiesParsed;
}

QVariantMap RequestPrivate::paramsMultiMapToVariantMap(const ParamsMultiMap &params)
{
    QVariantMap ret;
    auto end = params.constEnd();
    while (params.constBegin() != end) {
        --end;
        ret.insert(ret.constBegin(), end.key(), end.value());
    }
    return ret;
}

#include "moc_request.cpp"
