/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef ENGINE_UWSGI_H
#define ENGINE_UWSGI_H

#include <Cutelyst/engine.h>

#include <uwsgi.h>

#include <QPluginLoader>
#include <QLoggingCategory>
#include <QThread>

extern struct uwsgi_server uwsgi;

namespace Cutelyst {
class Dispatcher;
class Application;
}

using namespace Cutelyst;
class uWSGI : public Engine
{
    Q_OBJECT
public:
    explicit uWSGI(const QVariantMap &opts, Application *app, QObject *parent = 0);
    virtual ~uWSGI();

    virtual int workerId() const;

    virtual int workerCore() const;

    void setWorkerId(int id);
    void setWorkerCore(int core);

    void setThread(QThread *thread);

    virtual bool init() Q_DECL_FINAL;

    virtual bool finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData) override;

    virtual qint64 doWrite(Context *c, const char *data, qint64 len, void *engineData) Q_DECL_FINAL;

    void readRequestUWSGI(wsgi_request *req);

    void processRequest(wsgi_request *req);

    void addUnusedRequest(wsgi_request *wsgi_req);

    uwsgi_socket *watchSocket(struct uwsgi_socket *uwsgi_sock);

    uwsgi_socket *watchSocketAsync(struct uwsgi_socket *uwsgi_sock);

    /**
     * This method is called when an engine
     * fails to start on a thread so that we (main thread)
     * can reuse it's core requests
     */
    void reuseEngineRequests(uWSGI *engine);

    void stop();

    QVector<wsgi_request *> unusedRequestQueue() const;

    virtual quint64 time();

Q_SIGNALS:
    void postFork();
    void enableSockets(bool enable);

    /**
     * emitted when forked() fails
     */
    void engineDisabled(uWSGI *engine);

private:
    inline void validateAndExecuteRequest(wsgi_request *wsgi_req, int status);
    void forked();

    Cutelyst::Application *m_app;
    QVector<struct wsgi_request *> m_unusedReq;
    int m_workerId = 0;
    int m_workerCore = 0;
};

Q_DECLARE_LOGGING_CATEGORY(CUTELYST_UWSGI)

#endif // ENGINE_UWSGI_H
