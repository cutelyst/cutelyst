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
class RequestPrivate;
class Response;
class EnginePrivate;
class Headers;
class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(const QVariantHash &opts, QObject *parent = 0);
    virtual ~Engine();

    /**
     * @brief app
     * @return the Application object we are dealing with
     */
    Application *app() const;

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
     * Engine options
     */
    QVariantHash opts() const;

    /**
     * @brief user configuration for the application
     * @param entity the entity you are interested in
     * @return the configuration settings
     */
    QVariantHash config(const QString &entity) const;

    QByteArray statusCode(quint16 status) const;

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

    /**
     * Returns a time to be used for stats,
     * the default implementation returns
     * MSecsSinceEpoch, but if the engine
     * supports a more precise value it
     * can reimplement this method
     */
    virtual quint64 time();

    /**
     * Called by Response to manually write data
     */
    qint64 write(Context *c, const char *data, qint64 len);
protected:

    virtual qint64 doWrite(Context *c, const char *data, qint64 len, void *engineData) = 0;

    /**
     * Reimplement if you need a custom way
     * to Set-Cookie, the default implementation
     * writes them to c->res()->headers()
     */
    virtual void finalizeCookies(Context *c);

    /**
     * Finalize the headers, and call
     * doFinalizeHeaders(), reimplemententions
     * must call this first
     */
    virtual bool finalizeHeaders(Context *c);

    /**
     * Engines must reimplement this to write the
     * response body back to the caller
     */
    virtual void finalizeBody(Context *c, QIODevice *body, void *engineData) = 0;

    /**
     * Engines should overwrite this if they
     * want to to make custom error messages.
     * Default implementation render an html
     * with errors.
     */
    virtual void finalizeError(Context *c);

    /**
     * Engines must call this when the Request/Response objects
     * are ready for to be processed
     */
    void handleRequest(Request *request, bool autoDelete);

    /**
     * Called by Application to deal
     * with finalizing cookies, headers and body
     */
    void finalize(Context *c);

    EnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Engine)
    friend class Application;
    friend class Response;

    /**
     * @brief init the engine
     * @return true if succeeded
     */
    virtual bool init() = 0;
};

}

#endif // CUTELYST_ENGINE_H
