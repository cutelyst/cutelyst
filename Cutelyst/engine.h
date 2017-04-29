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

#ifndef CUTELYST_ENGINE_H
#define CUTELYST_ENGINE_H

#include <QObject>
#include <QHostAddress>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Headers>

namespace Cutelyst {

struct EngineRequest {
    /** The method used (GET, POST...) */
    QString method;
    /** The path requested by the user agent '/index' */
    QString path;
    /** The query string requested by the user agent 'foo=bar&baz' */
    QByteArray query;
    /** The protocol requested by the user agent 'HTTP1/1' */
    QString protocol;
    /** The server address which the server is listening to,
     *  usually the 'Host' header but if that's not present should be filled with the server address */
    QString serverAddress;
    /** The remote/client address */
    QHostAddress remoteAddress;
    /** The remote user name set by a front web server */
    QString remoteUser;
    /** The request headers */
    Headers headers;
    /** The timestamp of the start of headers */
    quint64 startOfRequest;
    /** The QIODevice containing the body (if any) of the request */
    QIODevice *body;
    /** The internal pointer of the request, to be used for mapping this request to the real request */
    void *requestPtr;
    /** The remote/client port */
    quint16 remotePort;
    /** If the connection is secure HTTPS */
    bool isSecure;
};

class Application;
class Context;
class EnginePrivate;
class CUTELYST_LIBRARY Engine : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs an Engine object, where \p app is the application that might be
     * used to create new instances if \p workerCore is greater than 1, \p opts
     * is the options loaded by the engine subclass.
     */
    explicit Engine(Application *app, int workerCore, const QVariantMap &opts);
    virtual ~Engine();

    /**
     * Returns the application associated with this engine.
     */
    Application *app() const;

    /**
     * Reimplement this to get the workerId of the engine subclass, this is
     * the same as processes id.
     */
    virtual int workerId() const = 0;

    /**
     * Returns the worker core set when constructing the engine
     */
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

    /**
     * Sets the configuration to be used by Application
     */
    void setConfig(const QVariantMap &config);

    /**
     * Returns a QVariantMap with the INI parsed from \p filename.
     */
    static QVariantMap loadIniConfig(const QString &filename);

    /**
     * Returns a QVariantMap with the JSON parsed from \p filename.
     */
    static QVariantMap loadJsonConfig(const QString &filename);

    /**
     * @return current micro seconds time to be used for stats, the default implementation returns
     * QDateTime::currentMSecsSinceEpoch() * 1000, to become micro seconds, so if the engine
     * supports a more precise value it can reimplement this method.
     */
    virtual quint64 time();

protected:
    /**
     * @brief initApplication
     *
     * This method inits the application and
     * calls init on the engine. It must be called on the
     * engine's thread
     *
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

    /**
     * Reimplement this to do the RAW writing to the client
     */
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
     * Reimplement this to write the headers back to the client
     */
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
        bool lastWasLetter = false;
        for (int i = 0 ; i < key.size() ; ++i) {
            QCharRef c = key[i];
            if (c == QLatin1Char('_')) {
                c = QLatin1Char('-');
                lastWasLetter = false;
            } else if (lastWasLetter) {
                c = c.toLower();
            } else if (c.isLetter()) {
                lastWasLetter = true;
            }
        }
        return key;
    }

    /**
     * Convert Header key to camel case
     */
    static inline void camelCaseByteArrayHeader(QByteArray &key) {
        // The RFC 2616 and 7230 states keys are not case
        // case sensitive, however several tools fail
        // if the headers are not on camel case form.
        bool lastWasLetter = false;
        for (int i = 0 ; i < key.size() ; ++i) {
            QByteRef c = key[i];
            if (c == '_') {
                c = '-';
                lastWasLetter = false;
            } else if (lastWasLetter) {
                c = QChar::toLower(c);
            } else if (QChar::isLetter(c)) {
                lastWasLetter = true;
            }
        }
    }

    /**
     * Returns the HTTP status message for the give \p status.
     */
    static const char *httpStatusMessage(quint16 status, int *len = nullptr);

    /**
     * This is the HTTP default response headers that each request gets
     */
    Headers &defaultHeaders();

    /**
     * Process the EngineRequest \p req
     */
    void processRequest(const EngineRequest &req);

    Q_DECL_DEPRECATED
    /**
     * Deprecated
     */
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
