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
    explicit EngineUwsgi(int coreId, Application *app = 0);
    ~EngineUwsgi();

    void setThread(QThread *thread);

    virtual bool init();

    virtual void finalizeHeaders(Context *ctx);
    virtual void finalizeBody(Context *ctx);

    void readRequestUWSGI(wsgi_request *req);

    void processRequest(wsgi_request *req);

    inline QByteArray httpCase(char *key, int key_len) const;

    virtual void reload();

    void addUnusedRequest(wsgi_request *req);
    void watchSocket(struct uwsgi_socket *uwsgi_sock);

Q_SIGNALS:
    void postFork();

private:
    void forked();

    int m_coreId;
    Cutelyst::Application *m_app;
    QList<struct wsgi_request *> m_unusedReq;
    QByteArray m_headerContentType = QByteArray("Content-Type", 12);
    QByteArray m_headerContentEncoding = QByteArray("Content-Encoding", 16);
    QByteArray m_headerConnectionKey = QByteArray("Connection", 10);
    QByteArray m_headerConnectionValue = QByteArray("HTTP/1.1", 8);
};

Q_DECLARE_LOGGING_CATEGORY(CUTELYST_UWSGI)

#endif // ENGINE_UWSGI_H
