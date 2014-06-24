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

#ifndef CUTELYST_ENGINE_H
#define CUTELYST_ENGINE_H

#include <QObject>
#include <QHostAddress>
#include <QUrlQuery>
#include <QFile>

namespace Cutelyst {

class Application;
class Context;
class Request;
class Response;
class EnginePrivate;
class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    /**
     * @brief finalizeCookies first called if no error
     * @param ctx
     * Reimplement if you need a custom way
     * to Set-Cookie, the default implementation
     * writes them to ctx->res()->headers()
     */
    virtual void finalizeCookies(Context *ctx);

    /**
     * @brief finalizeHeaders called after finalizeCookies
     * @param ctx
     * Engines must reimplement this to write response
     * headers back to the caller
     */
    virtual void finalizeHeaders(Context *ctx) = 0;

    /**
     * @brief finalizeBody called after finalizeHeaders
     * @param ctx
     * Engines must reimplement this to write the
     * response body back to the caller
     */
    virtual void finalizeBody(Context *ctx) = 0;

    /**
     * @brief finalizeError called on error
     * @param ctx
     * Engines should overwrite this if they
     * want to to make custom error messages.
     * Default implementation render an html
     * with errors.
     */
    virtual void finalizeError(Context *ctx);

    /**
     * @brief app
     * @return the Application object we are dealing with
     */
    Application *app() const;

    /**
     * @brief initApplication
     *
     * This method inits the application and
     * calls init on the engine.
     *
     * @param app the Application to init
     * @param postFork when true it will call postFork on the application
     * @return true if succeded
     */
    bool initApplication(Application *app, bool postFork);

    /**
     * @brief postForkApplication
     *
     * Should be called after the engine forks
     *
     * @return true if the engine should use this
     * process
     */
    bool postForkApplication();

    static QByteArray statusCode(quint16 status);

    /**
     * @brief reload
     *
     * Reloads the engine, in some engines
     * this means restarting the worker process
     * others might just ignore.
     *
     * Default implementation does nothing.
     *
     * \warning Use with care as this might not
     * properly clean your process.
     */
    virtual void reload();

    /**
     * @brief user configuration for the application
     * @param entity the entity you are interested in
     * @return the configuration settings
     */
    QVariantHash config(const QString &entity) const;

protected:
    /**
     * @brief handleRequest
     * @param request
     * @param response
     * Engines must call this when the Request/Response objects
     * are ready for to be processed
     */
    void handleRequest(Request *request, Response *response, bool autoDelete = true);
    Request *newRequest(void *requestData,
                        const QByteArray &scheme,
                        const QByteArray &hostAndPort,
                        const QByteArray &path,
                        const QUrlQuery &queryString);
    void setupRequest(Request *request,
                      const QByteArray &method,
                      const QByteArray &protocol,
                      const QHash<QByteArray, QByteArray> &headers,
                      QIODevice *body,
                      const QByteArray &remoteUser,
                      const QHostAddress &address,
                      quint16 peerPort);
    void *requestPtr(Request *request) const;

protected:
    EnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Engine)

    /**
     * @brief init the engine
     * @return true if succeeded
     */
    virtual bool init() = 0;
};

}

#endif // CUTELYST_ENGINE_H
