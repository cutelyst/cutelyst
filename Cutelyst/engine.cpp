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
#include <QDebug>

using namespace Cutelyst;

Engine::Engine(const QVariantHash &opts, QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate)
{
    Q_D(Engine);

    // Load application configuration
    if (qEnvironmentVariableIsSet("CUTELYST_CONFIG")) {
        QByteArray config = qgetenv("CUTELYST_CONFIG");
        qCDebug(CUTELYST_CORE) << "Reading config file:" << config;
        QSettings settings(config, QSettings::IniFormat);
        Q_FOREACH (const QString &group, settings.childGroups()) {
            settings.beginGroup(group);
            Q_FOREACH (const QString &key, settings.childKeys()) {
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
    Q_FOREACH (const QNetworkCookie &cookie, res->cookies()) {
        headers.pushHeader(QStringLiteral("Set-Cookie"), cookie.toRawForm());
    }
}

bool Engine::finalizeHeaders(Context *c)
{
    Response *response = c->response();

    // Check if we already finalized headers
    if (response->d_ptr->finalizedHeaders) {
        return true;
    }

    // Fix missing content length
    if (response->hasBody() && !response->contentLength()) {
        response->setContentLength(response->body().size());
    }

    const QString &protocol = c->request()->protocol();
    if (protocol == QLatin1String("HTTP/1.1")) {
        const QString &te = response->header(QStringLiteral("Transfer-Encoding"));
        if (te == QLatin1String("chunked")) {
            qCDebug(CUTELYST_ENGINE, "Chunked transfer-encoding set for response");
            c->d_ptr->chunked = true;
        } else if (!response->contentLength()) {
            qCDebug(CUTELYST_ENGINE, "Using chunked transfer-encoding to send unknown length body");
            response->setHeader(QStringLiteral("Transfer-Encoding"), QStringLiteral("chunked"));
            c->d_ptr->chunked = true;
        }
    }

    // Handle redirects
    const QUrl &location = response->location();
    if (!location.isEmpty()) {
        qCDebug(CUTELYST_ENGINE, "Redirecting to \"%s\"", location.toEncoded().data());
        response->headers().setHeader(QStringLiteral("Location"), location.toEncoded());
    }

    finalizeCookies(c);

    // Done
    response->d_ptr->finalizedHeaders = true;
    return true;
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

    res->body() = body;

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
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

qint64 Engine::write(Context *c, const char *data, qint64 len)
{
    void *engineData = c->engineData();
    if (c->d_ptr->chunked) {
        char chunked[19];
        int ret = snprintf(chunked, 19, "%X\r\n", (unsigned int) len);
        if (ret <= 0 || ret >= 19) {
            return -1;
        }
        doWrite(c, chunked, ret, engineData);
    }
    return doWrite(c, data, len, engineData);
}

QByteArray Engine::statusCode(quint16 status) const
{
    switch (status) {
    case Response::OK:
        return QByteArrayLiteral("200 OK");
    case Response::Found:
        return QByteArrayLiteral("302 Found");
    case Response::NotFound:
        return QByteArrayLiteral("404 Not Found");
    case Response::InternalServerError:
        return QByteArrayLiteral("500 Internal Server Error");
    case Response::MovedPermanently:
        return QByteArrayLiteral("301 Moved Permanently");
    case Response::NotModified:
        return QByteArrayLiteral("304 Not Modified");
    case Response::SeeOther:
        return QByteArrayLiteral("303 See Other");
    case Response::Forbidden:
        return QByteArrayLiteral("403 Forbidden");
    case Response::TemporaryRedirect:
        return QByteArrayLiteral("307 Temporary Redirect");
    case Response::Unauthorized:
        return QByteArrayLiteral("401 Unauthorized");
    case Response::BadRequest:
        return QByteArrayLiteral("400 Bad Request");
    case Response::MethodNotAllowed:
        return QByteArrayLiteral("405 Method Not Allowed");
    case Response::RequestTimeout:
        return QByteArrayLiteral("408 Request Timeout");
    case Response::Continue:
        return QByteArrayLiteral("100 Continue");
    case Response::SwitchingProtocols:
        return QByteArrayLiteral("101 Switching Protocols");
    case Response::Created:
        return QByteArrayLiteral("201 Created");
    case Response::Accepted:
        return QByteArrayLiteral("202 Accepted");
    case Response::NonAuthoritativeInformation:
        return QByteArrayLiteral("203 Non-Authoritative Information");
    case Response::NoContent:
        return QByteArrayLiteral("204 No Content");
    case Response::ResetContent:
        return QByteArrayLiteral("205 Reset Content");
    case Response::PartialContent:
        return QByteArrayLiteral("206 Partial Content");
    case Response::MultipleChoices:
        return QByteArrayLiteral("300 Multiple Choices");
    case Response::UseProxy:
        return QByteArrayLiteral("305 Use Proxy");
    case Response::PaymentRequired:
        return QByteArrayLiteral("402 Payment Required");
    case Response::NotAcceptable:
        return QByteArrayLiteral("406 Not Acceptable");
    case Response::ProxyAuthenticationRequired:
        return QByteArrayLiteral("407 Proxy Authentication Required");
    case Response::Conflict:
        return QByteArrayLiteral("409 Conflict");
    case Response::Gone:
        return QByteArrayLiteral("410 Gone");
    case Response::LengthRequired:
        return QByteArrayLiteral("411 Length Required");
    case Response::PreconditionFailed:
        return QByteArrayLiteral("412 Precondition Failed");
    case Response::RequestEntityTooLarge:
        return QByteArrayLiteral("413 Request Entity Too Large");
    case Response::RequestURITooLong:
        return QByteArrayLiteral("414 Request-URI Too Long");
    case Response::UnsupportedMediaType:
        return QByteArrayLiteral("415 Unsupported Media Type");
    case Response::RequestedRangeNotSatisfiable:
        return QByteArrayLiteral("416 Requested Range Not Satisfiable");
    case Response::ExpectationFailed:
        return QByteArrayLiteral("417 Expectation Failed");
    case Response::NotImplemented:
        return QByteArrayLiteral("501 Not Implemented");
    case Response::BadGateway:
        return QByteArrayLiteral("502 Bad Gateway");
    case Response::ServiceUnavailable:
        return QByteArrayLiteral("503 Service Unavailable");
    case Response::GatewayTimeout:
        return QByteArrayLiteral("504 Gateway Timeout");
    case Response::HTTPVersionNotSupported:
        return QByteArrayLiteral("505 HTTP Version Not Supported");
    case Response::BandwidthLimitExceeded:
        return QByteArrayLiteral("509 Bandwidth Limit Exceeded");
    default:
        return QByteArray::number(status);
    }
}

void Engine::reload()
{
    qCWarning(CUTELYST_ENGINE) << "Default reload implementation called, doing nothing";
}

QVariantHash Engine::opts() const
{
    Q_D(const Engine);
    return d->opts;
}

QVariantHash Engine::config(const QString &entity) const
{
    Q_D(const Engine);
    return d->config.value(entity);
}

void Engine::handleRequest(Request *request, bool autoDelete)
{
    Q_D(Engine);
    Q_ASSERT(d->app);
    d->app->handleRequest(request);

    if (autoDelete) {
        delete request;
    }
}

void Engine::finalize(Context *c)
{
    if (c->error()) {
        finalizeError(c);
    }

    Response *response = c->response();

    if (!response->d_ptr->finalizedHeaders) {
        finalizeHeaders(c);
    }

    QIODevice *body = response->bodyDevice();
    if (body) {
        finalizeBody(c, body, c->engineData());
    }
}
