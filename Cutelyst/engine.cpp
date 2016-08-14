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
#include <QThread>
#include <QByteArray>
#include <QDebug>

using namespace Cutelyst;

Engine::Engine(Cutelyst::Application *app, int workerCore, const QVariantMap &opts)
    : d_ptr(new EnginePrivate)
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
    d->workerCore = workerCore;

    // If workerCore is greater than 0 we need a new application instance
    if (workerCore) {
        auto newApp = qobject_cast<Application *>(app->metaObject()->newInstance());
        if (!newApp) {
            qFatal("*** FATAL *** Could not create a NEW instance of your Cutelyst::Application, "
                   "make sure your constructor has Q_INVOKABLE macro or disable threaded mode.");
        }
        d->app = newApp;
    } else {
        d->app = app;
    }

    // To make easier for engines to clean up
    // the app must be a child of it
    d->app->setParent(this);
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

int Engine::workerCore() const
{
    Q_D(const Engine);
    return d->workerCore;
}

bool Engine::initApplication()
{
    Q_D(Engine);

    if (thread() != QThread::currentThread()) {
        qCCritical(CUTELYST_ENGINE) << "Cannot init application on a different thread";
        return false;
    }

    if (!d->app->setup(this)) {
        qCCritical(CUTELYST_ENGINE) << "Failed to setup application";
        return false;
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

const char *Engine::httpStatusMessage(quint16 status, int *len)
{
    const char *ret;
    switch (status) {
    case Response::OK:
        ret = "200 OK";
        break;
    case Response::Found:
        ret = "302 Found";
        break;
    case Response::NotFound:
        ret = "404 Not Found";
        break;
    case Response::InternalServerError:
        ret = "500 Internal Server Error";
        break;
    case Response::MovedPermanently:
        ret = "301 Moved Permanently";
        break;
    case Response::NotModified:
        ret = "304 Not Modified";
        break;
    case Response::SeeOther:
        ret = "303 See Other";
        break;
    case Response::Forbidden:
        ret = "403 Forbidden";
        break;
    case Response::TemporaryRedirect:
        ret = "307 Temporary Redirect";
        break;
    case Response::Unauthorized:
        ret = "401 Unauthorized";
        break;
    case Response::BadRequest:
        ret = "400 Bad Request";
        break;
    case Response::MethodNotAllowed:
        ret = "405 Method Not Allowed";
        break;
    case Response::RequestTimeout:
        ret = "408 Request Timeout";
        break;
    case Response::Continue:
        ret = "100 Continue";
        break;
    case Response::SwitchingProtocols:
        ret = "101 Switching Protocols";
        break;
    case Response::Created:
        ret = "201 Created";
        break;
    case Response::Accepted:
        ret = "202 Accepted";
        break;
    case Response::NonAuthoritativeInformation:
        ret = "203 Non-Authoritative Information";
        break;
    case Response::NoContent:
        ret = "204 No Content";
        break;
    case Response::ResetContent:
        ret = "205 Reset Content";
        break;
    case Response::PartialContent:
        ret = "206 Partial Content";
        break;
    case Response::MultipleChoices:
        ret = "300 Multiple Choices";
        break;
    case Response::UseProxy:
        ret = "305 Use Proxy";
        break;
    case Response::PaymentRequired:
        ret = "402 Payment Required";
        break;
    case Response::NotAcceptable:
        ret = "406 Not Acceptable";
        break;
    case Response::ProxyAuthenticationRequired:
        ret = "407 Proxy Authentication Required";
        break;
    case Response::Conflict:
        ret = "409 Conflict";
        break;
    case Response::Gone:
        ret = "410 Gone";
        break;
    case Response::LengthRequired:
        ret = "411 Length Required";
        break;
    case Response::PreconditionFailed:
        ret = "412 Precondition Failed";
        break;
    case Response::RequestEntityTooLarge:
        ret = "413 Request Entity Too Large";
        break;
    case Response::RequestURITooLong:
        ret = "414 Request-URI Too Long";
        break;
    case Response::UnsupportedMediaType:
        ret = "415 Unsupported Media Type";
        break;
    case Response::RequestedRangeNotSatisfiable:
        ret = "416 Requested Range Not Satisfiable";
        break;
    case Response::ExpectationFailed:
        ret = "417 Expectation Failed";
        break;
    case Response::NotImplemented:
        ret = "501 Not Implemented";
        break;
    case Response::BadGateway:
        ret = "502 Bad Gateway";
        break;
    case Response::ServiceUnavailable:
        ret = "503 Service Unavailable";
        break;
    case Response::GatewayTimeout:
        ret = "504 Gateway Timeout";
        break;
    case Response::HTTPVersionNotSupported:
        ret = "505 HTTP Version Not Supported";
        break;
    case Response::BandwidthLimitExceeded:
        ret = "509 Bandwidth Limit Exceeded";
        break;
    default:
        ret = QByteArray::number(status).constData();
        break;
    }

    if (len) {
        *len = strlen(ret);
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
