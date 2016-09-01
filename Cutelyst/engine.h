/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Application;
class Context;
class Headers;
class EnginePrivate;
class CUTELYST_LIBRARY Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(Application *app, int workerCore, const QVariantMap &opts);
    virtual ~Engine();

    Application *app() const;

    virtual int workerId() const = 0;

    int workerCore() const;

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

    virtual quint64 time();

protected:
    /**
     * @brief initApplication
     *
     * This method inits the application and
     * calls init on the engine. It must be called on the
     * engine's thread
     *
     * @param app the Application to init
     * @param postFork when true it will call postFork on the application
     * @return true if succeded
     */
    bool initApplication();

    /**
     * @brief postForkApplication
     *
     * Subclasses must be call after the engine forks by the worker thread,
     * if no forking is involved it must be called once the worker thread has
     * started.
     *
     * For convenience QThread::currentThread() has it's object name set with
     * the worker core number.
     *
     * @return true if the engine should use this process
     */
    bool postForkApplication();

    /**
     * Called by Response to manually write data
     */
    qint64 write(Context *c, const char *data, qint64 len, void *engineData);

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

    virtual bool finalizeHeadersWrite(Context *c, quint16 status,  const Headers &headers, void *engineData) = 0;

    /**
     * Engines must reimplement this to write the
     * response body back to the caller
     */
    virtual void finalizeBody(Context *c);

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

    static inline void camelCaseByteArrayHeader(QByteArray &key) {
        // The RFC 2616 and 7230 states keys are not case
        // case sensitive, however several tools fail
        // if the headers are not on camel case form.
        bool lastWasDash = true;
        for (int i = 0 ; i < key.size() ; ++i) {
            QByteRef c = key[i];
            if (c == '_') {
                c = '-';
                lastWasDash = true;
            } else if (lastWasDash) {
                lastWasDash = false;
                c = QChar::toUpper(c);
            }
        }
    }

    static const char *httpStatusMessage(quint16 status, int *len = nullptr);

    void processRequest(const QString &method,
                        const QString &path,
                        const QByteArray &query,
                        const QString &protocol,
                        bool https,
                        const QString &serverAddress,
                        const QHostAddress &remoteAddress,
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
