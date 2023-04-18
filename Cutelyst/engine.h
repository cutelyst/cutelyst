/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_ENGINE_H
#define CUTELYST_ENGINE_H

#include <Cutelyst/Headers>
#include <Cutelyst/cutelyst_global.h>

#include <QHostAddress>
#include <QObject>

namespace Cutelyst {

class Application;
class Context;
class EngineRequest;
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

    /**
     * Process the EngineRequest \p req, the caller
     * must delete the context when the request is finished.
     *
     * This method allows for engines to keep the Context alive
     * while processing websocket data.
     */
    void processRequest(EngineRequest *request);

    /**
     * Returns the header key in camel case form
     */
    static inline QString camelCaseHeader(const QString &headerKey)
    {
        // The RFC 2616 and 7230 states keys are not case
        // case sensitive, however several tools fail
        // if the headers are not on camel case form.
        QString key        = headerKey;
        bool lastWasLetter = false;
        for (int i = 0; i < key.size(); ++i) {
            QChar c = key[i];
            if (c == u'_') {
                key[i]        = u'-';
                lastWasLetter = false;
            } else if (lastWasLetter) {
                key[i] = c.toLower();
            } else if (c.isLetter()) {
                lastWasLetter = true;
            }
        }
        return key;
    }

    /**
     * Convert Header key to camel case
     */
    static inline void camelCaseByteArrayHeader(QByteArray &key)
    {
        // The RFC 2616 and 7230 states keys are not case
        // case sensitive, however several tools fail
        // if the headers are not on camel case form.
        bool lastWasLetter = false;
        for (int i = 0; i < key.size(); ++i) {
            char c = key[i];
            if (c == '_') {
                key[i]        = '-';
                lastWasLetter = false;
            } else if (lastWasLetter) {
                key[i] = QChar::toLower(c);
            } else if (QChar::isLetter(c)) {
                lastWasLetter = true;
            }
        }
    }

    /**
     * Returns the HTTP status message for the given \p status.
     */
    static const char *httpStatusMessage(quint16 status, int *len = nullptr);

Q_SIGNALS:
    /**
     * Process the EngineRequest \p req Async, the caller
     * must delete the context when the request is finished.
     *
     * This method allows for engines to keep the Context alive
     * while processing websocket data.
     */
    void processRequestAsync(Cutelyst::EngineRequest *request);

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
     * This is the HTTP default response headers that each request gets
     */
    Headers &defaultHeaders();

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

inline bool Engine::isZeroWorker() const
{
    return !workerId() && !workerCore();
}

} // namespace Cutelyst

#endif // CUTELYST_ENGINE_H
