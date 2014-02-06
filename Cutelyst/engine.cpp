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

#include "request_p.h"
#include "application.h"
#include "response.h"
#include "context_p.h"

#include <QUrl>
#include <QHostInfo>
#include <QDebug>

using namespace Cutelyst;

typedef QPair<QString, QString> StringPair;

Engine::Engine(QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate)
{
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
    foreach (const StringPair &queryItem, queryString.queryItems()) {
        requestPriv->queryParam.insertMulti(queryItem.first, queryItem.second);
    }

    return new Request(requestPriv);
}

void Engine::setupRequest(Request *request, const QByteArray &method, const QByteArray &protocol, const QHash<QByteArray, QByteArray> &headers, const QByteArray &body, const QByteArray &remoteUser, const QHostAddress &address, quint16 peerPort, QFile *upload)
{
    request->d_ptr->method = method;
    request->d_ptr->protocol = protocol;
    request->d_ptr->body = body;
    request->d_ptr->headers = headers;
    request->d_ptr->remoteUser = remoteUser;
    request->d_ptr->address = address;
    request->d_ptr->port = peerPort;
    request->d_ptr->upload = upload;

    QByteArray cookies = headers.value("Cookie");
    request->d_ptr->cookies = QNetworkCookie::parseCookies(cookies.replace(';', '\n'));

    if (request->contentType() == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) of type "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        QMultiHash<QString, QString> bodyParam;
        foreach (const QByteArray &parameter, body.split('&')) {
            if (parameter.isEmpty()) {
                continue;
            }

            QList<QByteArray> parts = parameter.split('=');
            if (parts.size() == 2) {
                QByteArray value = parts.at(1);
                value.replace('+', ' ');
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                      QUrl::fromPercentEncoding(value));
            } else {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                                      QString());
            }
        }
        request->d_ptr->bodyParam = bodyParam;
    }
    request->d_ptr->param = request->d_ptr->bodyParam + request->d_ptr->queryParam;
}

void *Engine::requestPtr(Request *request) const
{
    return request->d_ptr->requestPtr;
}

void Engine::finalizeCookies(Context *ctx)
{
    foreach (const QNetworkCookie &cookie, ctx->response()->cookies()) {
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

bool Engine::setupApplication(Application *app)
{
    Q_D(Engine);
    d->app = app;
    app->setup(this);
    return init();
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

void Engine::handleRequest(Request *request, Response *response, bool autoDelete)
{
    Q_D(Engine);
    Q_ASSERT(d->app);
    d->app->handleRequest(request, response);

    if (autoDelete) {
        delete request;
        delete response;
    }
}

