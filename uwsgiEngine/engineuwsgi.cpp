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

#include "engineuwsgi.h"

#include "bodyuwsgi.h"
#include "bodybuffereduwsgi.h"

#include <QtCore/QSocketNotifier>
#include <QtCore/QCoreApplication>

#include <Cutelyst/common.h>
#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request_p.h>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI, "cutelyst.uwsgi")

using namespace Cutelyst;

typedef struct {
    QFile *bodyFile;
    BodyUWSGI *bodyUWSGI;
    BodyBufferedUWSGI *bodyBufferedUWSGI;
    RequestPrivate *priv;
    Request *request;
} CachedRequest;

uWSGI::uWSGI(const QVariantHash &opts, Application *app, QObject *parent) : Engine(opts, parent)
  , m_app(app)
{
    connect(this, &uWSGI::postFork,
            this, &uWSGI::forked);
}

uWSGI::~uWSGI()
{
}

void uWSGI::setThread(QThread *thread)
{
    moveToThread(thread);
    connect(thread, &QThread::started,
            this, &uWSGI::forked, Qt::DirectConnection);
}

qint64 uWSGI::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    if (uwsgi_response_write_body_do(static_cast<wsgi_request*>(engineData),
                                     const_cast<char *>(data),
                                     len) != UWSGI_OK) {
        qCWarning(CUTELYST_UWSGI) << "Failed to write body";
        return -1;
    }
    return len;
}

void uWSGI::readRequestUWSGI(wsgi_request *wsgi_req)
{
    for(;;) {
        int ret = uwsgi_wait_read_req(wsgi_req);
        if (ret <= 0) {
            qCDebug(CUTELYST_UWSGI) << "Failed wait read.";
            goto end;
        }

        int status = wsgi_req->socket->proto(wsgi_req);
        if (status < 0) {
            qCDebug(CUTELYST_UWSGI) << "Failed broken socket.";
            goto end;
        } else if (status == 0) {
            break;
        }
    }

    // empty request ?
    if (!wsgi_req->uh->pktsize) {
        qCDebug(CUTELYST_UWSGI) << "Empty request. skip.";
        goto end;
    }

    // get uwsgi variables
    if (uwsgi_parse_vars(wsgi_req)) {
        // If static maps are set or there is some error
        // this returns -1 so we just close the request
        goto end;
    }

    processRequest(wsgi_req);

end:
    uwsgi_close_request(wsgi_req);
}

static inline uint16_t notSlash(char *str, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        if (str[i] != '/') {
            return i;
        }
    }
    return length;
}

void uWSGI::processRequest(wsgi_request *req)
{
    CachedRequest *cache = static_cast<CachedRequest *>(req->async_environ);

    RequestPrivate *priv = cache->priv;
    priv->reset();

    priv->startOfRequest = req->start_of_request;
    priv->https = req->https_len;
    // wsgi_req->uri containg the whole URI it /foo/bar?query=null
    // so we use path_info, maybe it would be better to just build our
    // Request->uri() from it, but we need to run a performance test
    uint16_t pos = notSlash(req->path_info, req->path_info_len);
    priv->path = QString::fromLatin1(req->path_info + pos, req->path_info_len - pos);

    priv->serverAddress = QString::fromLatin1(req->host, req->host_len);
    priv->query = QByteArray::fromRawData(req->query_string, req->query_string_len);

    priv->method = QString::fromLatin1(req->method, req->method_len);
    priv->protocol = QString::fromLatin1(req->protocol, req->protocol_len);
    priv->remoteAddress = QHostAddress(QString::fromLatin1(req->remote_addr, req->remote_addr_len));
    priv->remoteUser = QString::fromLatin1(req->remote_user, req->remote_user_len);

    uint16_t remote_port_len;
    char *remote_port = uwsgi_get_var(req, (char *) "REMOTE_PORT", 11, &remote_port_len);
    priv->remotePort = QByteArray::fromRawData(remote_port, remote_port_len).toUInt();

    Headers headers;
    for (int i = 0; i < req->var_cnt; i += 2) {
        if (req->hvec[i].iov_len < 6) {
            continue;
        }

        if (!uwsgi_startswith((char *) req->hvec[i].iov_base,
                              const_cast<char *>("HTTP_"), 5)) {
            headers.setHeader(QString::fromLatin1((char *) req->hvec[i].iov_base+5, req->hvec[i].iov_len-5),
                              QString::fromLatin1((char *) req->hvec[i + 1].iov_base, req->hvec[i + 1].iov_len));
        }
    }

    if (req->content_type_len > 0) {
        headers.setContentType(QString::fromLatin1(req->content_type, req->content_type_len));
    }

    if (req->encoding_len > 0) {
        headers.setContentEncoding(QString::fromLatin1(req->encoding, req->encoding_len));
    }
    priv->headers = headers;

    QIODevice *body;
    if (req->post_file) {
//        qCDebug(CUTELYST_UWSGI) << "Post file available:" << req->post_file;
        QFile *upload = cache->bodyFile;
        if (upload->open(req->post_file, QIODevice::ReadOnly)) {
            body = upload;
        } else {
//            qCDebug(CUTELYST_UWSGI) << "Could not open post file:" << upload->errorString();
            body = cache->bodyBufferedUWSGI;
        }
    } else if (uwsgi.post_buffering) {
//        qCDebug(CUTELYST_UWSGI) << "Post buffering size:" << uwsgi.post_buffering;
        body = cache->bodyUWSGI;
    } else {
        // BodyBufferedUWSGI is an IO device which will
        // only consume the body when some of it's functions
        // is called, this is because here we can't seek
        // the body.
        body = cache->bodyBufferedUWSGI;
    }
    priv->body = body;

    handleRequest(cache->request, false);

    body->close();
}

void uWSGI::reload()
{
    qCDebug(CUTELYST_UWSGI) << "Reloading application due application request";
    uwsgi_reload(uwsgi.argv);
}

void uWSGI::addUnusedRequest(wsgi_request *wsgi_req)
{
    CachedRequest *cache = static_cast<CachedRequest *>(wsgi_req->async_environ);
    if (cache) {
        // TODO move to a class
        delete cache;
    }
    cache = new CachedRequest;
    cache->bodyFile = new QFile(this);
    cache->bodyUWSGI = new BodyUWSGI(wsgi_req, this);
    cache->bodyBufferedUWSGI = new BodyBufferedUWSGI(wsgi_req, this);
    cache->priv = new RequestPrivate;
    cache->priv->engine = this;
    cache->priv->requestPtr = wsgi_req;
    cache->request = new Request(cache->priv);
    wsgi_req->async_environ = cache;

    m_unusedReq.append(wsgi_req);
}

void uWSGI::watchSocket(struct uwsgi_socket *uwsgi_sock)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(uwsgi_sock->fd, QSocketNotifier::Read, this);
    connect(this, &uWSGI::enableSockets,
            socketNotifier, &QSocketNotifier::setEnabled);
    connect(socketNotifier, &QSocketNotifier::activated,
            [=](int fd) {
        struct wsgi_request *wsgi_req = m_unusedReq.takeFirst();
        if (wsgi_req == NULL) {
            uwsgi_async_queue_is_full(uwsgi_now());
            return;
        }

        // fill wsgi_request structure
        wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock);

        // mark core as used
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;

        // accept the connection
        if (wsgi_req_simple_accept(wsgi_req, fd) != 0) {
            uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
            m_unusedReq.append(wsgi_req);
            return;
        }

        wsgi_req->start_of_request = uwsgi_micros();
        wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

#ifdef UWSGI_GO_CHEAP_CODE
        // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
            set_harakiri(uwsgi.harakiri_options.workers);
        }
#endif // UWSGI_GO_CHEAP_CODE

        CachedRequest *cache = static_cast<CachedRequest *>(wsgi_req->async_environ);
        readRequestUWSGI(wsgi_req);
        wsgi_req->async_environ = cache;

        m_unusedReq.append(wsgi_req);
    });
}

void uWSGI::reuseEngineRequests(uWSGI *engine)
{
    Q_FOREACH (struct wsgi_request *req, engine->unusedRequestQueue()) {
        addUnusedRequest(req);
    }
}

void uWSGI::stop()
{
    Q_EMIT enableSockets(false);

    if (thread() != qApp->thread()) {
        thread()->quit();
    }
}

QList<wsgi_request *> uWSGI::unusedRequestQueue() const
{
    return m_unusedReq;
}

quint64 uWSGI::time()
{
    return uwsgi_micros();
}

bool uWSGI::finalizeHeaders(Context *ctx)
{
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->request()->engineData());
    Response *res = ctx->res();

    QByteArray status = statusCode(res->status());
    if (uwsgi_response_prepare_headers(wsgi_req,
                                       status.data(),
                                       status.size())) {
        return false;
    }

    if (!Engine::finalizeHeaders(ctx)) {
        return false;
    }

    const Headers &headers = res->headers();
    QHash<QString, QString>::ConstIterator it = headers.constBegin();
    QHash<QString, QString>::ConstIterator end = headers.constEnd();
    while (it != end) {
        QByteArray key = camelCaseHeader(it.key()).toLatin1();
        QByteArray value = it.value().toLatin1();

        if (uwsgi_response_add_header(wsgi_req,
                                      key.data(),
                                      key.size(),
                                      value.data(),
                                      value.size())) {
            return false;
        }

        ++it;
    }

    return true;
}

bool uWSGI::init()
{
    return true;
}

void uWSGI::forked()
{
    if (QThread::currentThread() != qApp->thread()) {
        m_app = qobject_cast<Application *>(m_app->metaObject()->newInstance());
        if (!m_app) {
            uwsgi_log("*** FATAL *** Could not create a NEW instance of your Cutelyst::Application, "
                      "make sure your constructor has Q_INVOKABLE macro.\n");
            Q_EMIT engineDisabled(this);
            return;
        }

        // Move the application and it's children to this thread
        m_app->moveToThread(thread());

        // We can now set a parent
        m_app->setParent(this);

        // init and postfork
        if (!initApplication(m_app, true)) {
            uwsgi_log("Failed to init application on a different thread than main.\n");
            Q_EMIT engineDisabled(this);
            return;
        }
    } else if (!postForkApplication()) {
#ifdef UWSGI_GO_CHEAP_CODE
        // We need to tell the master process that the
        // application failed to setup and that it shouldn't
        // try to respawn the worker
        exit(UWSGI_GO_CHEAP_CODE);
#endif // UWSGI_GO_CHEAP_CODE
    }

    if (!m_unusedReq.isEmpty()) {
        // Start Monitoring Sockets
        struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
        while(uwsgi_sock) {
            watchSocket(uwsgi_sock);
            uwsgi_sock = uwsgi_sock->next;
        }
    }
}
