/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include <QJsonDocument>

using namespace Cutelyst;

/*!
 \class Cutelyst::Engine
 \brief The Cutelyst Engine

 This class is responsible receiving the request
 and sending the response. It must be reimplemented
 by real HTTP engines due some pure virtual methods.

 The subclass must create an Engine per thread (worker core),
 if the Application passed to the constructor has a worker core
 greater than 0 it will issue a new Application instance, failing
 to do so a fatal error is generated (usually indicating that
 the Application does not have a Q_INVOKABLE constructor).
*/

/*!
 @param app The application loaded
 @param workerCore The thread number
 @param opts The configuation options
 */
Engine::Engine(Cutelyst::Application *app, int workerCore, const QVariantMap &opts)
    : d_ptr(new EnginePrivate)
{
    Q_D(Engine);

    // Debug messages should be disabled by default
    QLoggingCategory::setFilterRules(QLatin1String("cutelyst.*.debug=false"));

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
        headers.pushHeader(QStringLiteral("set_cookie"), QString::fromLatin1(cookie.toRawForm()));
    }
}

bool Engine::finalizeHeaders(Context *c)
{
    Response *response = c->response();
    quint16 status = response->status();
    Headers &headers = response->headers();

    // Fix missing content length
    if (headers.contentLength() < 0) {
        qint64 size = response->size();
        if (size >= 0) {
            headers.setContentLength(size);
        }
    }

    finalizeCookies(c);

    // Done
    response->d_ptr->flags |= ResponsePrivate::FinalizedHeaders;
    return finalizeHeadersWrite(c, status, headers, c->request()->engineData());
}

void Engine::finalizeBody(Context *c)
{
    Response *response = c->response();
    void *engineData = c->engineData();

    if (!(response->d_ptr->flags & ResponsePrivate::Chunked)) {
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
    } else if (!(response->d_ptr->flags & ResponsePrivate::ChunkedDone)) {
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

/**
 * @brief application
 * @return the Application object we are dealing with
 */
Application *Engine::app() const
{
    Q_D(const Engine);
    Q_ASSERT(d->app);
    return d->app;
}

/*! \fn virtual int Engine::workerId() const = 0

 The id is the number of the spawned engine process,
 a single process workerId = 0, two process 0 for the first
 1 for the second.

 \note the value returned from this function is
 only valid when postFork() is issued.

 \returns the worker id (process)
*/

/*!
 Each worker process migth have a number of worker cores (threads),
 a single process with two worker threads will return 0 and 1 for
 each of the thread respectively.

 \returns the worker core (thread)
*/
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

    QThread::currentThread()->setObjectName(QString::number(d->workerCore));

    return d->app->enginePostFork();
}

quint64 Engine::time()
{
    return QDateTime::currentMSecsSinceEpoch() * 1000;
}

qint64 Engine::write(Context *c, const char *data, qint64 len, void *engineData)
{
    Response *response = c->response();
    if (!(response->d_ptr->flags & ResponsePrivate::Chunked)) {
        return doWrite(c, data, len, engineData);
    } else if (!(response->d_ptr->flags & ResponsePrivate::ChunkedDone)) {
        const QByteArray chunkSize = QByteArray::number(len, 16).toUpper();
        QByteArray chunk;
        chunk.reserve(len + chunkSize.size() + 4);
        chunk.append(chunkSize).append("\r\n", 2)
                .append(data, len).append("\r\n", 2);

        qint64 retWrite = doWrite(c, chunk.data(), chunk.size(), engineData);

        // Flag if we wrote an empty chunk
        if (!len) {
            response->d_ptr->flags |= ResponsePrivate::ChunkedDone;
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
        ret = "HTTP/1.1 200 OK";
        break;
    case Response::Found:
        ret = "HTTP/1.1 302 Found";
        break;
    case Response::NotFound:
        ret = "HTTP/1.1 404 Not Found";
        break;
    case Response::InternalServerError:
        ret = "HTTP/1.1 500 Internal Server Error";
        break;
    case Response::MovedPermanently:
        ret = "HTTP/1.1 301 Moved Permanently";
        break;
    case Response::NotModified:
        ret = "HTTP/1.1 304 Not Modified";
        break;
    case Response::SeeOther:
        ret = "HTTP/1.1 303 See Other";
        break;
    case Response::Forbidden:
        ret = "HTTP/1.1 403 Forbidden";
        break;
    case Response::TemporaryRedirect:
        ret = "HTTP/1.1 307 Temporary Redirect";
        break;
    case Response::Unauthorized:
        ret = "HTTP/1.1 401 Unauthorized";
        break;
    case Response::BadRequest:
        ret = "HTTP/1.1 400 Bad Request";
        break;
    case Response::MethodNotAllowed:
        ret = "HTTP/1.1 405 Method Not Allowed";
        break;
    case Response::RequestTimeout:
        ret = "HTTP/1.1 408 Request Timeout";
        break;
    case Response::Continue:
        ret = "HTTP/1.1 100 Continue";
        break;
    case Response::SwitchingProtocols:
        ret = "HTTP/1.1 101 Switching Protocols";
        break;
    case Response::Created:
        ret = "HTTP/1.1 201 Created";
        break;
    case Response::Accepted:
        ret = "HTTP/1.1 202 Accepted";
        break;
    case Response::NonAuthoritativeInformation:
        ret = "HTTP/1.1 203 Non-Authoritative Information";
        break;
    case Response::NoContent:
        ret = "HTTP/1.1 204 No Content";
        break;
    case Response::ResetContent:
        ret = "HTTP/1.1 205 Reset Content";
        break;
    case Response::PartialContent:
        ret = "HTTP/1.1 206 Partial Content";
        break;
    case Response::MultipleChoices:
        ret = "HTTP/1.1 300 Multiple Choices";
        break;
    case Response::UseProxy:
        ret = "HTTP/1.1 305 Use Proxy";
        break;
    case Response::PaymentRequired:
        ret = "HTTP/1.1 402 Payment Required";
        break;
    case Response::NotAcceptable:
        ret = "HTTP/1.1 406 Not Acceptable";
        break;
    case Response::ProxyAuthenticationRequired:
        ret = "HTTP/1.1 407 Proxy Authentication Required";
        break;
    case Response::Conflict:
        ret = "HTTP/1.1 409 Conflict";
        break;
    case Response::Gone:
        ret = "HTTP/1.1 410 Gone";
        break;
    case Response::LengthRequired:
        ret = "HTTP/1.1 411 Length Required";
        break;
    case Response::PreconditionFailed:
        ret = "HTTP/1.1 412 Precondition Failed";
        break;
    case Response::RequestEntityTooLarge:
        ret = "HTTP/1.1 413 Request Entity Too Large";
        break;
    case Response::RequestURITooLong:
        ret = "HTTP/1.1 414 Request-URI Too Long";
        break;
    case Response::UnsupportedMediaType:
        ret = "HTTP/1.1 415 Unsupported Media Type";
        break;
    case Response::RequestedRangeNotSatisfiable:
        ret = "HTTP/1.1 416 Requested Range Not Satisfiable";
        break;
    case Response::ExpectationFailed:
        ret = "HTTP/1.1 417 Expectation Failed";
        break;
    case Response::NotImplemented:
        ret = "HTTP/1.1 501 Not Implemented";
        break;
    case Response::BadGateway:
        ret = "HTTP/1.1 502 Bad Gateway";
        break;
    case Response::ServiceUnavailable:
        ret = "HTTP/1.1 503 Service Unavailable";
        break;
    case Response::GatewayTimeout:
        ret = "HTTP/1.1 504 Gateway Timeout";
        break;
    case Response::HTTPVersionNotSupported:
        ret = "HTTP/1.1 505 HTTP Version Not Supported";
        break;
    case Response::BandwidthLimitExceeded:
        ret = "HTTP/1.1 509 Bandwidth Limit Exceeded";
        break;
    default:
        ret = QByteArrayLiteral("HTTP/1.1 ").append(QByteArray::number(status)).constData();
        break;
    }

    if (len) {
        *len = strlen(ret);
    }
    return ret;
}

Headers &Engine::defaultHeaders()
{
    Q_D(Engine);
    return d->app->defaultHeaders();
}

Context *Engine::processRequest2(const EngineRequest &req)
{
    Q_D(Engine);

    auto request = new Request(new RequestPrivate(req, this));
    return d->app->handleRequest2(request);
}

Context *Engine::processRequest3(EngineConnection *conn)
{

}

void Engine::processRequest(const EngineRequest &req)
{
    delete processRequest2(req);
}

QVariantMap Engine::opts() const
{
    Q_D(const Engine);
    return d->opts;
}

QVariantMap Engine::config(const QString &entity) const
{
    Q_D(const Engine);
    return d->config.value(entity).toMap();
}

void Engine::setConfig(const QVariantMap &config)
{
    Q_D(Engine);
    d->config = config;
}

QVariantMap Engine::loadIniConfig(const QString &filename)
{
    QVariantMap ret;
    QSettings settings(filename, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        qCWarning(CUTELYST_ENGINE) << "Failed to load INI file:" << settings.status();
        return ret;
    }

    const auto groups = settings.childGroups();
    for (const QString &group : groups) {
        QVariantMap configGroup;
        settings.beginGroup(group);
        const auto child = settings.childKeys();
        for (const QString &key : child) {
            configGroup.insert(key, settings.value(key));
        }
        settings.endGroup();
        ret.insert(group, configGroup);
    }

    return ret;
}

QVariantMap Engine::loadJsonConfig(const QString &filename)
{
    QVariantMap ret;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return ret;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    ret = doc.toVariant().toMap();

    return ret;
}

void Engine::finalize(Context *c)
{
    if (c->error()) {
        finalizeError(c);
    }

    if (!(c->response()->d_ptr->flags & ResponsePrivate::FinalizedHeaders) && !finalizeHeaders(c)) {
        return;
    }

    finalizeBody(c);
}

bool Engine::webSocketHandshake(Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    ResponsePrivate *priv = c->response()->d_ptr;
    if (priv->flags & ResponsePrivate::FinalizedHeaders) {
        return false;
    }

    if (webSocketHandshakeDo(c, key, origin, protocol, c->engineData())) {
        priv->flags |= ResponsePrivate::FinalizedHeaders;
        return true;
    }

    return false;
}

bool Engine::webSocketHandshakeDo(Context *c, const QString &key, const QString &origin, const QString &protocol, void *engineData)
{
    Q_UNUSED(c)
    Q_UNUSED(key)
    Q_UNUSED(origin)
    Q_UNUSED(protocol)
    Q_UNUSED(engineData)
    return false;
}

bool Engine::webSocketSendTextMessage(Context *c, const QString &message)
{
    Q_UNUSED(c)
    Q_UNUSED(message)
    return false;
}

bool Engine::webSocketSendBinaryMessage(Context *c, const QByteArray &message)
{
    Q_UNUSED(c)
    Q_UNUSED(message)
    return false;
}

bool Engine::webSocketSendPing(Context *c, const QByteArray &payload)
{
    Q_UNUSED(c)
    Q_UNUSED(payload)
    return false;
}

bool Engine::webSocketClose(Context *c, quint16 code, const QString &reason)
{
    Q_UNUSED(c)
    Q_UNUSED(code)
    Q_UNUSED(reason)
    return false;
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

    EngineRequest req;
    req.method = method;
    req.path = path;
    req.query = query;
    req.protocol = protocol;
    req.isSecure = isSecure;
    req.serverAddress = serverAddress;
    req.remoteAddress = remoteAddress;
    req.remotePort = remotePort;
    req.remoteUser = remoteUser;
    req.headers = headers;
    req.startOfRequest = startOfRequest;
    req.body = body;
    req.requestPtr = requestPtr;

    auto request = new Request(new RequestPrivate(req, this));
    delete d->app->handleRequest2(request);
}

#include "moc_engine.cpp"
