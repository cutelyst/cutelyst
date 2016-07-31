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

#include "engine_p.h"

#include "common.h"
#include "request_p.h"
#include "application.h"
#include "response_p.h"
#include "context_p.h"

#include <QUrl>
#include <QSettings>
#include <QDir>
#include <QtCore/QByteArray>
#include <QDebug>

using namespace Cutelyst;

Engine::Engine(const QVariantMap &opts, QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate)
{
    Q_D(Engine);

    // Debug messages should be disabled by default
    QLoggingCategory::setFilterRules(QLatin1String("cutelyst.*.debug=false"));

    // Load application configuration
    if (qEnvironmentVariableIsSet("CUTELYST_CONFIG")) {
        const QByteArray config = qgetenv("CUTELYST_CONFIG");
        qCDebug(CUTELYST_CORE) << "Reading config file:" << config;
        QSettings settings(QString::fromLatin1(config), QSettings::IniFormat);
        const auto groups = settings.childGroups();
        for (const QString &group : groups) {
            settings.beginGroup(group);
            const auto child = settings.childKeys();
            for (const QString &key : child) {
                d->config[group].insert(key, settings.value(key));
            }
            settings.endGroup();
        }
        qCDebug(CUTELYST_CORE) << "Configuration:" << d->config;
    }

    d->opts = opts;
}

Engine::~Engine()
{
    delete d_ptr;
}

void Engine::finalizeCookies(Context *c)
{
    Response *res = c->response();
    Headers &headers = res->headers();
    const auto cookies = res->cookies();
    for (const QNetworkCookie &cookie : cookies) {
        headers.pushHeader(QStringLiteral("Set-Cookie"), QString::fromLatin1(cookie.toRawForm()));
    }
}

bool Engine::finalizeHeaders(Context *c)
{
    Response *response = c->response();
    quint16 status = response->status();
    Headers &headers = response->headers();

    // Fix missing content length
    if (!response->contentLength() && response->hasBody()) {
        QIODevice *body = response->bodyDevice();
        if (!body) {
            response->setContentLength(response->body().size());
        } else {
            qint64 size = body->size();
            if (size >= 0) {
                response->setContentLength(body->size());
            } else if (c->request()->protocol() == QLatin1String("HTTP/1.1")) {
                // if status is not 1xx or 204 NoContent or 304 NotModified
                if (!(status >= 100 && status <= 199) && status != 204 && status != 304) {
                    qCDebug(CUTELYST_ENGINE, "Using chunked transfer-encoding to send unknown length body");
                    headers.setHeader(QStringLiteral("Transfer-Encoding"), QStringLiteral("chunked"));
                    response->d_ptr->chunked = true;
                }
            }
        }
    } else if (headers.header(QStringLiteral("Transfer-Encoding")) == QLatin1String("chunked")) {
        qCDebug(CUTELYST_ENGINE, "Chunked transfer-encoding set for response");
        response->d_ptr->chunked = true;
    }

    // Handle redirects
    const QUrl &location = response->location();
    if (!location.isEmpty()) {
        qCDebug(CUTELYST_ENGINE, "Redirecting to \"%s\"", location.toEncoded().constData());
        headers.setHeader(QStringLiteral("Location"), QString::fromLatin1(location.toEncoded()));
    }

    finalizeCookies(c);

    // Done
    response->d_ptr->finalizedHeaders = true;
    return finalizeHeadersWrite(c, status, headers, c->request()->engineData());
}

void Engine::finalizeBody(Context *c)
{
    Response *response = c->response();
    void *engineData = c->engineData();

    if (!response->d_ptr->chunked) {
        QIODevice *body = response->bodyDevice();

        if (body) {
            body->seek(0);
            char block[64 * 1024];
            while (!body->atEnd()) {
                qint64 in = body->read(block, sizeof(block));
                if (in <= 0) {
                    break;
                }

                if (write(c, block, in, engineData) != in) {
                    qCWarning(CUTELYST_ENGINE) << "Failed to write body";
                    break;
                }
            }
        } else {
            const QByteArray bodyByteArray = response->body();
            write(c, bodyByteArray.constData(), bodyByteArray.size(), engineData);
        }
    } else if (!response->d_ptr->chunked_done) {
        // Write the final '0' chunk
        doWrite(c, "0\r\n\r\n", 5, engineData);
    }
}

void Engine::finalizeError(Context *c)
{
    Response *res = c->response();

    res->setContentType(QStringLiteral("text/html; charset=utf-8"));

    QByteArray body;

    // Trick IE. Old versions of IE would display their own error page instead
    // of ours if we'd give it less than 512 bytes.
    body.reserve(512);

    body.append(c->errors().join(QLatin1Char('\n')).toUtf8());

    res->setBody(body);

    // Return 500
    res->setStatus(Response::InternalServerError);
}

Application *Engine::app() const
{
    Q_D(const Engine);
    Q_ASSERT(d->app);
    return d->app;
}

bool Engine::initApplication(Application *app, bool postFork)
{
    Q_D(Engine);
    d->app = app;

    // To make easier for engines to clean up
    // the app must be a child of it
    app->setParent(this);

    if (!app->setup(this)) {
        qCCritical(CUTELYST_ENGINE) << "Failed to setup application";
        return false;
    }

    if (!init()) {
        qCCritical(CUTELYST_ENGINE) << "Failed to setup engine";
        return false;
    }

    if (postFork) {
        return postForkApplication();
    }

    return true;
}

bool Engine::postForkApplication()
{
    Q_D(Engine);

    if (!d->app) {
        qCCritical(CUTELYST_ENGINE) << "Failed to postForkApplication on a null application";
        return false;
    }
    return d->app->enginePostFork();
}

quint64 Engine::time()
{
    return QDateTime::currentMSecsSinceEpoch();
}

qint64 Engine::write(Context *c, const char *data, qint64 len, void *engineData)
{
    Response *response = c->response();
    if (!response->d_ptr->chunked) {
        return doWrite(c, data, len, engineData);
    } else if (!response->d_ptr->chunked_done) {
        const QByteArray chunkSize = QByteArray::number(len, 16).toUpper();
        QByteArray chunk;
        chunk.reserve(len + chunkSize.size() + 4);
        chunk.append(chunkSize).append("\r\n", 2)
                .append(data, len).append("\r\n", 2);

        qint64 retWrite = doWrite(c, chunk.data(), chunk.size(), engineData);

        // Flag if we wrote an empty chunk
        if (!len) {
            response->d_ptr->chunked_done = true;
        }

        return retWrite == chunk.size() ? len : -1;
    }
    return -1;
}

QByteArray Engine::statusCode(quint16 status)
{
    QByteArray ret;
    switch (status) {
    case Response::OK:
        ret = QByteArrayLiteral("200 OK");
        break;
    case Response::Found:
        ret = QByteArrayLiteral("302 Found");
        break;
    case Response::NotFound:
        ret = QByteArrayLiteral("404 Not Found");
        break;
    case Response::InternalServerError:
        ret = QByteArrayLiteral("500 Internal Server Error");
        break;
    case Response::MovedPermanently:
        ret = QByteArrayLiteral("301 Moved Permanently");
        break;
    case Response::NotModified:
        ret = QByteArrayLiteral("304 Not Modified");
        break;
    case Response::SeeOther:
        ret = QByteArrayLiteral("303 See Other");
        break;
    case Response::Forbidden:
        ret = QByteArrayLiteral("403 Forbidden");
        break;
    case Response::TemporaryRedirect:
        ret = QByteArrayLiteral("307 Temporary Redirect");
        break;
    case Response::Unauthorized:
        ret = QByteArrayLiteral("401 Unauthorized");
        break;
    case Response::BadRequest:
        ret = QByteArrayLiteral("400 Bad Request");
        break;
    case Response::MethodNotAllowed:
        ret = QByteArrayLiteral("405 Method Not Allowed");
        break;
    case Response::RequestTimeout:
        ret = QByteArrayLiteral("408 Request Timeout");
        break;
    case Response::Continue:
        ret = QByteArrayLiteral("100 Continue");
        break;
    case Response::SwitchingProtocols:
        ret = QByteArrayLiteral("101 Switching Protocols");
        break;
    case Response::Created:
        ret = QByteArrayLiteral("201 Created");
        break;
    case Response::Accepted:
        ret = QByteArrayLiteral("202 Accepted");
        break;
    case Response::NonAuthoritativeInformation:
        ret = QByteArrayLiteral("203 Non-Authoritative Information");
        break;
    case Response::NoContent:
        ret = QByteArrayLiteral("204 No Content");
        break;
    case Response::ResetContent:
        ret = QByteArrayLiteral("205 Reset Content");
        break;
    case Response::PartialContent:
        ret = QByteArrayLiteral("206 Partial Content");
        break;
    case Response::MultipleChoices:
        ret = QByteArrayLiteral("300 Multiple Choices");
        break;
    case Response::UseProxy:
        ret = QByteArrayLiteral("305 Use Proxy");
        break;
    case Response::PaymentRequired:
        ret = QByteArrayLiteral("402 Payment Required");
        break;
    case Response::NotAcceptable:
        ret = QByteArrayLiteral("406 Not Acceptable");
        break;
    case Response::ProxyAuthenticationRequired:
        ret = QByteArrayLiteral("407 Proxy Authentication Required");
        break;
    case Response::Conflict:
        ret = QByteArrayLiteral("409 Conflict");
        break;
    case Response::Gone:
        ret = QByteArrayLiteral("410 Gone");
        break;
    case Response::LengthRequired:
        ret = QByteArrayLiteral("411 Length Required");
        break;
    case Response::PreconditionFailed:
        ret = QByteArrayLiteral("412 Precondition Failed");
        break;
    case Response::RequestEntityTooLarge:
        ret = QByteArrayLiteral("413 Request Entity Too Large");
        break;
    case Response::RequestURITooLong:
        ret = QByteArrayLiteral("414 Request-URI Too Long");
        break;
    case Response::UnsupportedMediaType:
        ret = QByteArrayLiteral("415 Unsupported Media Type");
        break;
    case Response::RequestedRangeNotSatisfiable:
        ret = QByteArrayLiteral("416 Requested Range Not Satisfiable");
        break;
    case Response::ExpectationFailed:
        ret = QByteArrayLiteral("417 Expectation Failed");
        break;
    case Response::NotImplemented:
        ret = QByteArrayLiteral("501 Not Implemented");
        break;
    case Response::BadGateway:
        ret = QByteArrayLiteral("502 Bad Gateway");
        break;
    case Response::ServiceUnavailable:
        ret = QByteArrayLiteral("503 Service Unavailable");
        break;
    case Response::GatewayTimeout:
        ret = QByteArrayLiteral("504 Gateway Timeout");
        break;
    case Response::HTTPVersionNotSupported:
        ret = QByteArrayLiteral("505 HTTP Version Not Supported");
        break;
    case Response::BandwidthLimitExceeded:
        ret = QByteArrayLiteral("509 Bandwidth Limit Exceeded");
        break;
    default:
        ret = QByteArray::number(status);
        break;
    }
    return ret;
}

QVariantMap Engine::opts() const
{
    Q_D(const Engine);
    return d->opts;
}

QVariantMap Engine::config(const QString &entity) const
{
    Q_D(const Engine);
    return d->config.value(entity);
}

void Engine::finalize(Context *c)
{
    if (c->error()) {
        finalizeError(c);
    }

    if (!c->response()->d_ptr->finalizedHeaders && !finalizeHeaders(c)) {
        return;
    }

    finalizeBody(c);
}

static const QByteArray cutelyst_header_order (
        // General headers
        "Cache-Control\n"
        "Connection\n"
        "Date\n"
        "Pragma\n"
        "Trailer\n"
        "Transfer-Encoding\n"
        "Upgrade\n"
        "Via\n"
        "Warning\n"
        // Request headers
        "Accept\n"
        "Accept-Charset\n"
        "Accept-Encoding\n"
        "Accept-Language\n"
        "Authorization\n"
        "Expect\n"
        "From\n"
        "Host\n"
        "If-Match\n"
        "If-Modified-Since\n"
        "If-None-Match\n"
        "If-Range\n"
        "If-Unmodified-Since\n"
        "Max-Forwards\n"
        "Proxy-Authorization\n"
        "Range\n"
        "Referer\n"
        "TE\n"
        "User-Agent\n"
        // Response headers
        "Accept-Ranges\n"
        "Age\n"
        "ETag\n"
        "Location\n"
        "Proxy-Authenticate\n"
        "Retry-After\n"
        "Server\n"
        "Vary\n"
        "WWW-Authenticate\n"
        // Entity headers
        "Allow\n"
        "Content-Encoding\n"
        "Content-Language\n"
        "Content-Length\n"
        "Content-Location\n"
        "Content-MD5\n"
        "Content-Range\n"
        "Content-Type\n"
        "Expires\n"
        "Last-Modified"
        );

bool httpGoodPracticeWeightSort(const HeaderValuePair &pair1, const HeaderValuePair &pair2)
{
    int index1 = pair1.weight;
    int index2 = pair2.weight;

    if (index1 != -1 && index2 != -1) {
        // Both items are in the headerOrder list
        return index1 < index2;
    } else if (index1 == -1 && index2 == -1) {
        // Noone of them are int the headerOrder list
        return false;
    }

    // if the pair1 is in the header list it should go first
    return index1 != -1;
}

QList<HeaderValuePair> Engine::headersForResponse(const Headers &headers)
{
    QList<HeaderValuePair> ret;

    auto headersMap = headers.map();
    auto it = headersMap.constBegin();
    while (it != headersMap.constEnd()) {
        HeaderValuePair pair;
        pair.key = camelCaseHeader(it.key());
        pair.value = it.value();
        pair.weight = cutelyst_header_order.indexOf(pair.key.toLatin1());

        ret.append(pair);
        ++it;
    }

    // Sort base on the "good practices" of HTTP RCF
    qSort(ret.begin(), ret.end(), &httpGoodPracticeWeightSort);

    return ret;
}

void Engine::processRequest(const QString &method,
                            const QString &path,
                            const QByteArray &query,
                            const QString &protocol,
                            bool isSecure,
                            const QString &serverAddress,
                            const QHostAddress &remoteAddress,
                            quint16 remotePort,
                            const QString &remoteUser,
                            const Headers &headers,
                            quint64 startOfRequest,
                            QIODevice *body,
                            void *requestPtr)
{
    Q_D(Engine);

    auto prv = new RequestPrivate(this,
                                  method,
                                  path,
                                  query,
                                  protocol,
                                  isSecure,
                                  serverAddress,
                                  remoteAddress,
                                  remotePort,
                                  remoteUser,
                                  headers,
                                  startOfRequest,
                                  body,
                                  requestPtr);

    auto request = new Request(prv);
    d->app->handleRequest(request);
    delete request;
}

#include "moc_engine.cpp"
