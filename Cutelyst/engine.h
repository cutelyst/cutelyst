/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

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
     * Constructs an %Engine object, where \a app is the application that might be
     * used to create new instances if \a workerCore is greater than 1, \a opts
     * is the options loaded by the engine subclass.
     */
    explicit Engine(Application *app, int workerCore, const QVariantMap &opts);
    /**
     * Destroys the %Engine object.
     */
    virtual ~Engine();

    /**
     * Returns the application associated with this engine.
     */
    [[nodiscard]] Application *app() const;

    /**
     * Reimplement this to get the workerId of the engine subclass, this is
     * the same as processes id.
     *
     * The id is the number of the spawned engine process, a single process
     * workerId = 0, two process 0 for the first 1 for the second.
     *
     * \note The value returned from this function is only valid when postFork() is issued.
     */
    [[nodiscard]] virtual int workerId() const = 0;

    /**
     * Returns the worker core set when constructing the engine.
     *
     * Each worker process migth have a number of worker cores (threads),
     * a single process with two worker threads will return 0 and 1 for
     * each of the thread respectively.
     */
    [[nodiscard]] int workerCore() const;

    /**
     * Returns true if this is the Zero worker,
     * ie if workerId() == 0 and workerCore() == 0.
     *
     * \note the value returned from this function is
     * only valid when postFork() is issued.
     */
    [[nodiscard]] inline bool isZeroWorker() const;

    /**
     * Returns the engine options set in the constructor.
     */
    [[nodiscard]] QVariantMap opts() const;

    /**
     * Returns a map of key value pairs for the configuration \a entitiy (section) from
     * your application’s configuration file.
     *
     * \sa \ref configuration
     */
    [[nodiscard]] QVariantMap config(const QString &entity) const;

    /**
     * Sets the configuration to be used by Application.
     */
    void setConfig(const QVariantMap &config);

    /**
     * Returns a QVariantMap with the INI parsed from \a filename.
     */
    [[nodiscard]] static QVariantMap loadIniConfig(const QString &filename);

    /**
     * Returns a QVariantMap with the JSON parsed from \a filename.
     */
    [[nodiscard]] static QVariantMap loadJsonConfig(const QString &filename);

    /**
     * Process the \a request. The caller must delete the context when the request is finished.
     *
     * This method allows for engines to keep the Context alive while processing websocket data.
     */
    void processRequest(EngineRequest *request);

    /**
     * Returns the HTTP status message for the given \a status. If \a len is not a \c nullptr,
     * the length of the returned string will be stored to \a *len.
     */
    Q_DECL_DEPRECATED_X("Will be removed in new major release")
    static const char *httpStatusMessage(quint16 status, int *len = nullptr);

Q_SIGNALS:
    /**
     * Process the \a requst asynchronous. The caller
     * must delete the context when the request is finished.
     *
     * This method allows for engines to keep the Context alive
     * while processing websocket data.
     */
    void processRequestAsync(Cutelyst::EngineRequest *request);

protected:
    /**
     * This method inits the application and calls init on the engine. It must be called on the
     * engine's thread. Returns \c true on success.
     */
    bool initApplication();

    /**
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
     * This is the HTTP default response headers that each request gets.
     */
    [[nodiscard]] Headers &defaultHeaders();

    EnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Engine)
    friend class Application;
    friend class Response;

    /**
     * Initialize the engine and return \c true on success.
     */
    virtual bool init() = 0;
};

inline bool Engine::isZeroWorker() const
{
    return !workerId() && !workerCore();
}

} // namespace Cutelyst
