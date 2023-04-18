/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "common.h"
#include "context_p.h"
#include "engine_p.h"
#include "request_p.h"
#include "response_p.h"

#include <QByteArray>
#include <QDir>
#include <QJsonDocument>
#include <QSettings>
#include <QThread>
#include <QUrl>

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

    connect(this, &Engine::processRequestAsync, this, &Engine::processRequest, Qt::QueuedConnection);

    d->opts       = opts;
    d->workerCore = workerCore;
    d->app        = app;
}

Engine::~Engine()
{
    delete d_ptr;
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
    return quint64(QDateTime::currentMSecsSinceEpoch() * 1000);
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
    case Response::MultiStatus:
        ret = "HTTP/1.1 207 Multi-Status";
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
        *len = int(strlen(ret));
    }
    return ret;
}

Headers &Engine::defaultHeaders()
{
    Q_D(Engine);
    return d->app->defaultHeaders();
}

void Engine::processRequest(EngineRequest *request)
{
    Q_D(Engine);
    d->app->handleRequest(request);
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

#include "moc_engine.cpp"
