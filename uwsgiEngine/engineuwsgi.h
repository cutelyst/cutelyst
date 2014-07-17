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

#include "../Cutelyst/engine.h"

#include <uwsgi.h>

#include <QPluginLoader>
#include <QLoggingCategory>
#include <QThread>

#define free_req_queue uwsgi.async_queue_unused_ptr++; uwsgi.async_queue_unused[uwsgi.async_queue_unused_ptr] = wsgi_req

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
    explicit EngineUwsgi(Application *parent = 0);
    ~EngineUwsgi();

    void setThread(QThread *thread);

    virtual bool init();

    virtual void finalizeHeaders(Context *ctx);
    virtual void finalizeBody(Context *ctx);

    void readRequestUWSGI(wsgi_request *req);

    void processRequest(wsgi_request *req);

    QByteArray httpCase(const QByteArray &headerKey) const;

    virtual void reload();

Q_SIGNALS:
    void receiveRequest(wsgi_request *req);
    void postFork();
    void requestFinished(wsgi_request *wsgi_req);

private:
    void forked();
};

Q_DECLARE_LOGGING_CATEGORY(CUTELYST_UWSGI)

#endif // ENGINE_UWSGI_H
