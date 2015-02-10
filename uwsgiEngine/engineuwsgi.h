/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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
class EngineUwsgi : public Engine
{
    Q_OBJECT
public:
    explicit EngineUwsgi(const QVariantHash &opts, Application *app = 0);
    virtual ~EngineUwsgi();

    void setThread(QThread *thread);

    virtual bool init();

    virtual bool finalizeHeaders(Context *ctx, void *engineData);

    virtual void finalizeBody(Context *ctx, QIODevice *body, void *engineData);

    void readRequestUWSGI(wsgi_request *req);

    void processRequest(wsgi_request *req);

    inline QByteArray httpCase(char *key, int key_len) const;

    virtual void reload();

    void addUnusedRequest(wsgi_request *wsgi_req);

    void watchSocket(struct uwsgi_socket *uwsgi_sock);

    /**
     * This method is called when an engine
     * fails to start on a thread so that we (main thread)
     * can reuse it's core requests
     */
    void reuseEngineRequests(EngineUwsgi *engine);

    void stop();

    QList<struct wsgi_request *> unusedRequestQueue() const;

Q_SIGNALS:
    void postFork();
    void enableSockets(bool enable);

    /**
     * emitted when forked() fails
     */
    void engineDisabled(EngineUwsgi *engine);

private:
    void forked();

    Cutelyst::Application *m_app;
    QList<struct wsgi_request *> m_unusedReq;
};

Q_DECLARE_LOGGING_CATEGORY(CUTELYST_UWSGI)

#endif // ENGINE_UWSGI_H
