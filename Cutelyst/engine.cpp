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

    QUrl uri;
    if (hostAndPort.isEmpty()) {
        // This is a hack just in case remote is not set
        uri = scheme + "://" + QHostInfo::localHostName() + path;
    } else {
        uri = scheme + "://" + hostAndPort + path;
    }
    uri.setQuery(queryString);

    requestPriv->path = path;
    requestPriv->uri = uri;
    Q_FOREACH (const StringPair &queryItem, queryString.queryItems()) {
        requestPriv->queryParam.insertMulti(queryItem.first, queryItem.second);
    }

    return new Request(requestPriv);
}

void Engine::setupRequest(Request *request, const QByteArray &method, const QByteArray &protocol, const Headers &headers, QIODevice *body, const QByteArray &remoteUser, const QHostAddress &address, quint16 peerPort)
{
    request->d_ptr->method = method;
    request->d_ptr->protocol = protocol;
    request->d_ptr->body = body;
    request->d_ptr->headers = headers;
    request->d_ptr->remoteUser = remoteUser;
    request->d_ptr->address = address;
    request->d_ptr->port = peerPort;
}

void *Engine::requestPtr(Request *request) const
{
    return request->d_ptr->requestPtr;
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

QByteArray Engine::statusCode(quint16 status)
{
    QByteArray ret = QByteArray::number(status);
    switch (status) {
    case Response::OK:
        ret += " OK";
        break;
    case Response::MovedPermanently:
        ret += " Moved Permanently";
        break;
    case Response::Found:
        ret += " Found";
        break;
    case Response::NotModified:
        ret += " Not Modified";
        break;
    case Response::TemporaryRedirect:
        ret += " Temporary Redirect";
        break;
    case Response::BadRequest:
        ret += " Bad Request";
        break;
    case Response::AuthorizationRequired:
        ret += " Authorization Required";
        break;
    case Response::Forbidden:
        ret += " Forbidden";
        break;
    case Response::NotFound:
        ret += " Not Found";
        break;
    case Response::MethodNotAllowed:
        ret += " Method Not Allowed";
        break;
    case Response::InternalServerError:
        ret += " Internal Server Error";
        break;
    }

    return ret;
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

