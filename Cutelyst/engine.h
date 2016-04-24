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

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

typedef struct {
    int weight;
    QString key;
    QString value;
} HeaderValuePair;

class Application;
class Context;
class Request;
class Headers;
class EnginePrivate;
class RequestPrivate;
class CUTELYST_LIBRARY Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(const QVariantMap &opts, QObject *parent = nullptr);
    virtual ~Engine();

    /**
     * @brief app
     * @return the Application object we are dealing with
     */
    Application *app() const;

    /**
     * Returns the worker id (process)
     *
     * The id is the number of the spwaned engine process,
     * a single process workerId = 0, two process 0 for the first
     * 1 for the second.
     *
     * \note the value returned from this function is
     * only valid when postFork() is issued.
     */
    virtual int workerId() const = 0;

    /**
     * Returns the worker core (thread)
     *
     * Each worker process migth have a number of worker cores (threads),
     * a single process with two worker threads will return 0 and 1 for
     * each of the thread respectively.
     */
    virtual int workerCore() const = 0;

    /**
     * Returns true if this is the Zero worker,
     * ie if workerId() == 0 and workerCore() == 0
     *
     * \note the value returned from this function is
     * only valid when postFork() is issued.
     */
    inline bool isZeroWorker() const;

    /**
     * Engine options
     */
    QVariantMap opts() const;

    /**
     * @brief user configuration for the application
     * @param entity the entity you are interested in
     * @return the configuration settings
     */
    QVariantMap config(const QString &entity) const;

    static QByteArray statusCode(quint16 status);

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
     * doWriteHeader(), reimplemententions
     * must call this first
     */
    virtual bool finalizeHeaders(Context *c);

    /**
     * Engines must reimplement this to write the
     * response body back to the caller
     */
    virtual void finalizeBody(Context *c, QIODevice *body);

    /**
     * Engines should overwrite this if they
     * want to to make custom error messages.
     * Default implementation render an html
     * with errors.
     */
    virtual void finalizeError(Context *c);

    /**
     * Called by Application to deal
     * with finalizing cookies, headers and body
     */
    void finalize(Context *c);

    /**
     * Returns the hearder in the order suggested by HTTP RFC's
     * "good pratices", this function is mainly used by the Engine class
     */
    static QList<HeaderValuePair> headersForResponse(const Headers &headers);

    /**
     * Returns the header key in camel case form
     */
    static inline QString camelCaseHeader(const QString &headerKey) {
        // The RFC 2616 and 7230 states keys are not case
        // case sensitive, however several tools fail
        // if the headers are not on camel case form.
        QString key = headerKey;
        bool lastWasDash = true;
        for (int i = 0 ; i < key.size() ; ++i) {
            QCharRef c = key[i];
            if (c == QLatin1Char('_')) {
                c = QLatin1Char('-');
                lastWasDash = true;
            } else if (lastWasDash) {
                lastWasDash = false;
                c = c.toUpper();
            }
        }
        return key;
    }

    void processRequest(const QString &method,
                        const QString &path,
                        const QByteArray &query,
                        const QString &protocol,
                        bool https,
                        const QString &serverAddress,
                        const QString &remoteAddress,
                        quint16 remotePort,
                        const QString &remoteUser,
                        const Headers &headers,
                        quint64 startOfRequest,
                        QIODevice *body,
                        void *requestPtr);

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

inline bool Engine::isZeroWorker() const {
    return !workerId() && !workerCore();
}

}

#endif // CUTELYST_ENGINE_H
