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

#include "engine_p.h"

#include "common.h"
#include "request_p.h"
#include "application.h"
#include "response.h"
#include "context_p.h"

#include <QUrl>
#include <QHostInfo>
#include <QSettings>
#include <QDebug>

using namespace Cutelyst;

Engine::Engine(QObject *parent) :
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
}

Engine::~Engine()
{
    delete d_ptr;
}

void Engine::finalizeCookies(Context *ctx, void *engineData)
{
    Response *res = ctx->response();
    Q_FOREACH (const QNetworkCookie &cookie, res->cookies()) {
        res->addHeaderValue("Set-Cookie", cookie.toRawForm());
    }
}

void Engine::finalizeError(Context *ctx)
{
    Response *res = ctx->response();

    res->setContentType("text/html; charset=utf-8");

    QByteArray body;

    // Trick IE. Old versions of IE would display their own error page instead
    // of ours if we'd give it less than 512 bytes.
    body.reserve(512);

    body.append(ctx->errors().join(QLatin1Char('\n')).toUtf8());

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

void Engine::finalize(Context *ctx)
{
    if (ctx->error()) {
        finalizeError(ctx);
    }

    void *engineData = ctx->request()->engineData();
    finalizeCookies(ctx, engineData);

    Response *response = ctx->response();
    QIODevice *body = response->bodyDevice();
    if (body) {
        response->setContentLength(body->size());
    }

    if (finalizeHeaders(ctx, engineData)) {
        if (body) {
            finalizeBody(ctx, body, engineData);
        }
    }
}
