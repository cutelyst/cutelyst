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

#include "cutelystengine_p.h"

#include "cutelystrequest_p.h"
#include "cutelystresponse.h"
#include "context_p.h"

#include <QUrl>

using namespace Cutelyst;

Engine::Engine(QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate(this))
{
    Q_D(Engine);
}

Engine::~Engine()
{
    Q_D(Engine);

    delete d_ptr;
}

void Engine::createRequest(int connectionId, const QUrl &url, const QByteArray &method, const QString &protocol, const QHash<QString, QByteArray> &headers, const QByteArray &body)
{
    // Parse the query (GET) parameters ie "?foo=bar&bar=baz"
    QMultiHash<QString, QString> queryParam;
    foreach (const QString &parameter, url.query(QUrl::FullyEncoded).split(QLatin1Char('&'))) {
        if (parameter.isEmpty()) {
            continue;
        }

        QStringList parts = parameter.split(QLatin1Char('='));
        if (parts.size() == 2) {
            queryParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0).toUtf8()),
                                   QUrl::fromPercentEncoding(parts.at(1).toUtf8()));
        } else {
            queryParam.insertMulti(QUrl::fromPercentEncoding(parts.first().toUtf8()),
                                   QString());
        }
    }

    QMultiHash<QString, QString> bodyParam;
    if (headers.value(QLatin1String("Content-Type")) == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        foreach (const QByteArray &parameter, body.split('&')) {
            if (parameter.isEmpty()) {
                continue;
            }

            QList<QByteArray> parts = parameter.split('=');
            if (parts.size() == 2) {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                      QUrl::fromPercentEncoding(parts.at(1)));
            } else {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                                      QString());
            }
        }
    }

    QByteArray cookies = headers.value(QLatin1String("Cookie"));

    RequestPrivate *requestPriv = new RequestPrivate;
    requestPriv->engine = this;
    requestPriv->connectionId = connectionId;
    requestPriv->method = method;
    requestPriv->url = url;
    requestPriv->protocol = protocol;
    requestPriv->queryParam = queryParam;
    requestPriv->bodyParam = bodyParam;
    requestPriv->param = queryParam + bodyParam;
    requestPriv->headers = headers;
    requestPriv->cookies = QNetworkCookie::parseCookies(cookies.replace(';', '\n'));
    requestPriv->body = body;

    handleRequest(new Request(requestPriv), new Response);
}

EnginePrivate::EnginePrivate(Engine *parent) :
    q_ptr(parent)
{
}

EnginePrivate::~EnginePrivate()
{
}

void Engine::finalizeCookies(Context *ctx)
{
    foreach (const QNetworkCookie &cookie, ctx->response()->cookies()) {
        ctx->response()->addHeaderValue(QLatin1String("Set-Cookie"), cookie.toRawForm());
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
