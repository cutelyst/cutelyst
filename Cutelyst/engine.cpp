/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

typedef QPair<QString, QString> StringPair;

Engine::Engine(QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate)
{
    Q_D(Engine);

    // Load application configuration
    if (qEnvironmentVariableIsSet("CUTELYST_CONFIG")) {
        QByteArray config = qgetenv("CUTELYST_CONFIG");
        qCWarning(CUTELYST_CORE) << "Reading config file:" << config;
        QSettings settings(config, QSettings::IniFormat);
        Q_FOREACH (const QString &group, settings.childGroups()) {
            settings.beginGroup(group);
            QString groupLowered = group.toLower();
            Q_FOREACH (const QString &key, settings.childKeys()) {
                d->config[groupLowered].insert(key, settings.value(key));
            }
            settings.endGroup();
        }
    }
}

Engine::~Engine()
{
    delete d_ptr;
}

Request *Engine::newRequest(void *requestData, const QByteArray &scheme, const QByteArray &hostAndPort, const QByteArray &path, const QUrlQuery &queryString)
{
    RequestPrivate *requestPriv = new RequestPrivate;
    requestPriv->requestPtr = requestData;
    requestPriv->engine = this;
    requestPriv->path = QString::fromLocal8Bit(path);

    QUrl uri;
    if (hostAndPort.isEmpty()) {
        // This is a hack just in case remote is not set
        uri = scheme + "://" + QHostInfo::localHostName() + path;
    } else {
        uri = scheme + "://" + hostAndPort + path;
    }
    uri.setQuery(queryString);

    requestPriv->uri = uri;
    Q_FOREACH (const StringPair &queryItem, queryString.queryItems()) {
        requestPriv->queryParam.insertMulti(queryItem.first, queryItem.second);
    }

    return new Request(requestPriv);
}

void Engine::setupRequest(Request *request, const QByteArray &method, const QByteArray &protocol, const Headers &headers, QIODevice *body, const QByteArray &remoteUser, const QHostAddress &address, quint16 peerPort)
{
//    request->d_ptr->method = method;
//    request->d_ptr->protocol = protocol;
//    request->d_ptr->body = body;
//    request->d_ptr->headers = headers;
//    request->d_ptr->remoteUser = remoteUser;
//    request->d_ptr->address = address;
//    request->d_ptr->port = peerPort;
}

void Engine::finalizeCookies(Context *ctx)
{
    Q_FOREACH (const QNetworkCookie &cookie, ctx->response()->cookies()) {
        ctx->response()->addHeaderValue("Set-Cookie", cookie.toRawForm());
    }
}

void Engine::finalizeError(Context *ctx)
{
    ctx->res()->setContentType("text/html; charset=utf-8");

    QByteArray body;

    // Trick IE. Old versions of IE would display their own error page instead
    // of ours if we'd give it less than 512 bytes.
    body.reserve(512);

    body.append(ctx->errors().join(QLatin1Char('\n')));

    ctx->res()->body() = body;

    // Return 500
    ctx->res()->setStatus(Response::InternalServerError);
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
    return d->app->postFork();
}

QByteArray Engine::statusCode(quint16 status) const
{
    Q_D(const Engine);

    switch (status) {
    case Response::OK:
        return d->statusOk;
    case Response::Found:
        return d->statusFound;
    case Response::NotFound:
        return d->statusNotFound;
    case Response::InternalServerError:
        return "500 Internal Server Error";
    case Response::MovedPermanently:
        return "301 Moved Permanently";
    case Response::NotModified:
        return "304 Not Modified";
    case Response::SeeOther:
        return "303 See Other";
    case Response::Forbidden:
        return "403 Forbidden";
    case Response::TemporaryRedirect:
        return "307 Temporary Redirect";
    case Response::Unauthorized:
        return "401 Unauthorized";
    case Response::BadRequest:
        return "400 Bad Request";
    case Response::MethodNotAllowed:
        return "405 Method Not Allowed";
    case Response::RequestTimeout:
        return "408 Request Timeout";
    case Response::Continue:
        return "100 Continue";
    case Response::SwitchingProtocols:
        return "101 Switching Protocols";
    case Response::Created:
        return "201 Created";
    case Response::Accepted:
        return "202 Accepted";
    case Response::NonAuthoritativeInformation:
        return "203 Non-Authoritative Information";
    case Response::NoContent:
        return "204 No Content";
    case Response::ResetContent:
        return "205 Reset Content";
    case Response::PartialContent:
        return "206 Partial Content";
    case Response::MultipleChoices:
        return "300 Multiple Choices";
    case Response::UseProxy:
        return "305 Use Proxy";
    case Response::PaymentRequired:
        return "402 Payment Required";
    case Response::NotAcceptable:
        return "406 Not Acceptable";
    case Response::ProxyAuthenticationRequired:
        return "407 Proxy Authentication Required";
    case Response::Conflict:
        return "409 Conflict";
    case Response::Gone:
        return "410 Gone";
    case Response::LengthRequired:
        return "411 Length Required";
    case Response::PreconditionFailed:
        return "412 Precondition Failed";
    case Response::RequestEntityTooLarge:
        return "413 Request Entity Too Large";
    case Response::RequestURITooLong:
        return "414 Request-URI Too Long";
    case Response::UnsupportedMediaType:
        return "415 Unsupported Media Type";
    case Response::RequestedRangeNotSatisfiable:
        return "416 Requested Range Not Satisfiable";
    case Response::ExpectationFailed:
        return "417 Expectation Failed";
    case Response::NotImplemented:
        return "501 Not Implemented";
    case Response::BadGateway:
        return "502 Bad Gateway";
    case Response::ServiceUnavailable:
        return "503 Service Unavailable";
    case Response::GatewayTimeout:
        return "504 Gateway Timeout";
    case Response::HTTPVersionNotSupported:
        return "505 HTTP Version Not Supported";
    case Response::BandwidthLimitExceeded:
        return "509 Bandwidth Limit Exceeded";
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
    return d->config.value(entity.toLower());
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

Request *Engine::newRequest(RequestPrivate *priv)
{
    return new Request(priv);
}

