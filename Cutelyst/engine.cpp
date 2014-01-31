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
#include "response.h"
#include "context_p.h"

#include <QUrl>
#include <QDebug>

using namespace Cutelyst;

Engine::Engine(QObject *parent) :
    QObject(parent),
    d_ptr(new EnginePrivate(this))
{
}

Engine::~Engine()
{
    delete d_ptr;
}

void Engine::createRequest(void *data, const QUrl &url, const QByteArray &method, const QString &protocol, const QHash<QByteArray, QByteArray> &headers, const QByteArray &body)
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
    if (headers.value("Content-Type") == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
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
    }

    QByteArray cookies = headers.value("Cookie");

    RequestPrivate *requestPriv = new RequestPrivate;
    requestPriv->engine = this;
    requestPriv->connectionId = data;
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

QByteArray Engine::statusCode(quint16 status)
{
    QByteArray ret = QByteArray::number(status);
    qDebug() << ret << status;
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
