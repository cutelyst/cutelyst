/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef ENGINE_UWSGI_H
#define ENGINE_UWSGI_H

#include <Cutelyst/engine.h>

#include <QPluginLoader>
#include <QLoggingCategory>
#include <QThread>

extern struct uwsgi_server uwsgi;

struct wsgi_request;
struct uwsgi_socket;

namespace Cutelyst {
class Dispatcher;
class Application;
}

using namespace Cutelyst;
class uWSGI : public Engine
{
    Q_OBJECT
public:
    explicit uWSGI(Application *app, int workerCore, const QVariantMap &opts);
    virtual ~uWSGI();

    virtual int workerId() const override;

    void setWorkerId(int id);

    void setThread(QThread *thread);

    virtual bool init() final;

    void readRequestUWSGI(wsgi_request *req);

    void addUnusedRequest(wsgi_request *wsgi_req);

    uwsgi_socket *watchSocket(struct uwsgi_socket *uwsgi_sock);

    uwsgi_socket *watchSocketAsync(struct uwsgi_socket *uwsgi_sock);

    void stop();

    virtual quint64 time() override;

    bool forked();

Q_SIGNALS:
    void postFork();
    void enableSockets(bool enable);

private:
    friend class uwsgiConnection;

    inline void validateAndExecuteRequest(wsgi_request *wsgi_req, int status);

    std::vector<struct wsgi_request *> m_unusedReq;
    int m_workerId = 0;
};

Q_DECLARE_LOGGING_CATEGORY(CUTELYST_UWSGI)

#endif // ENGINE_UWSGI_H
