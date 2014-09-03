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

#include "engineuwsgi.h"

#include "bodyuwsgi.h"
#include "bodybuffereduwsgi.h"

#include <QSocketNotifier>

#include <Cutelyst/common.h>
#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Response>
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

EngineUwsgi::EngineUwsgi(Application *app) :
    m_app(app)
{
    connect(this, &EngineUwsgi::postFork,
            this, &EngineUwsgi::forked);
}

EngineUwsgi::~EngineUwsgi()
{
}

void EngineUwsgi::setThread(QThread *thread)
{
    moveToThread(thread);
    connect(thread, &QThread::started,
            this, &EngineUwsgi::forked, Qt::DirectConnection);
}

void EngineUwsgi::finalizeBody(Context *ctx, QIODevice *body, void *engineData)
{
    Q_UNUSED(ctx)

    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(engineData);
    body->seek(0);
    char block[4096];
    while (!body->atEnd()) {
        qint64 in = body->read(block, sizeof(block));
        if (in <= 0)
            break;

        if (uwsgi_response_write_body_do(wsgi_req, block, in) != UWSGI_OK) {
            qCWarning(CUTELYST_UWSGI) << "Failed to write body";
            break;
        }
    }
}

void EngineUwsgi::readRequestUWSGI(wsgi_request *wsgi_req)
{
    for(;;) {
        int ret = uwsgi_wait_read_req(wsgi_req);
        if (ret <= 0) {
            uwsgi_log("Failed wait read\n");
            goto end;
        }

        int status = wsgi_req->socket->proto(wsgi_req);
        if (status < 0) {
            uwsgi_log("Failed broken socket\n");
            goto end;
        } else if (status == 0) {
            break;
        }
    }

    // empty request ?
    if (!wsgi_req->uh->pktsize) {
        qCDebug(CUTELYST_UWSGI) << "Empty request. skip.";
        uwsgi_log("Failed empty request\n");
        goto end;
    }

    // get uwsgi variables
    if (uwsgi_parse_vars(wsgi_req)) {
        uwsgi_log("Invalid request. skip.\n");
        qCDebug(CUTELYST_UWSGI) << "Invalid request. skip.";
        goto end;
    }

    processRequest(wsgi_req);

    wsgi_req->end_of_request = uwsgi_micros();
    qCDebug(CUTELYST_STATS) << "Request took:" << ((wsgi_req->end_of_request-wsgi_req->start_of_request)/1000000.0) << "s";

end:
    uwsgi_close_request(wsgi_req);
}

void EngineUwsgi::processRequest(wsgi_request *req)
{
    CachedRequest *cache = static_cast<CachedRequest *>(req->async_environ);

    RequestPrivate *priv = cache->priv;
    priv->reset();

    priv->https = req->https_len;
    // wsgi_req->uri containg the whole URI it /foo/bar?query=null
    // so we use path_info, maybe it would be better to just build our
    // Request->uri() from it, but we need to run a performance test
    priv->path = QString::fromLatin1(req->path_info, req->path_info_len);

    char *pch = strchr(req->host, ':');
    if (pch) {
        priv->serverAddress = QString::fromLatin1(req->host, pch - req->host);
        priv->serverPort = QByteArray::fromRawData(req->host + (pch - req->host + 1), req->host_len - (pch - req->host + 1)).toUInt();
    } else {
        priv->serverAddress = QString::fromLatin1(req->host, req->host_len);
        priv->serverPort = 80;// fallback
    }
    priv->queryString = QString::fromLatin1(req->query_string, req->query_string_len);

    priv->method = QByteArray::fromRawData(req->method, req->method_len);
    priv->protocol = QByteArray::fromRawData(req->protocol, req->protocol_len);
    priv->remoteAddress = QHostAddress(QString::fromLatin1(req->remote_addr, req->remote_addr_len));
    priv->remoteUser = QByteArray::fromRawData(req->remote_user, req->remote_user_len);

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
            QByteArray key = httpCase((char *) req->hvec[i].iov_base+5, req->hvec[i].iov_len-5);
            QByteArray value = QByteArray::fromRawData((char *) req->hvec[i + 1].iov_base, req->hvec[i + 1].iov_len);
            headers.setHeader(key, value);
        }
    }

    if (req->content_type_len > 0) {
        headers.setHeader(m_headerContentType,
                          QByteArray::fromRawData(req->content_type, req->content_type_len));
    }

    if (req->encoding_len > 0) {
        headers.setHeader(m_headerContentEncoding,
                          QByteArray::fromRawData(req->encoding, req->encoding_len));
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

QByteArray EngineUwsgi::httpCase(char *key, int key_len) const
{
    bool lastWasUnderscore = true;
    for (int i = 0 ; i < key_len ; ++i) {
        QChar buf = key[i];
        if (buf == '_') {
            key[i] = '-';
            lastWasUnderscore = true;
        } else if(lastWasUnderscore) {
            lastWasUnderscore = false;
        } else {
            key[i] = buf.toLower().toLatin1();
        }
    }

    return QByteArray::fromRawData(key, key_len);
}

void EngineUwsgi::reload()
{
    qCDebug(CUTELYST_UWSGI) << "Reloading application due application request";
    uwsgi_reload(uwsgi.argv);
}

void EngineUwsgi::addUnusedRequest(wsgi_request *wsgi_req)
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

void EngineUwsgi::watchSocket(struct uwsgi_socket *uwsgi_sock)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(uwsgi_sock->fd, QSocketNotifier::Read, this);
    connect(this, &EngineUwsgi::enableSockets,
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
        if (wsgi_req_simple_accept(wsgi_req, fd)) {
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

void EngineUwsgi::reuseEngineRequests(EngineUwsgi *engine)
{
    Q_FOREACH (struct wsgi_request *req, engine->unusedRequestQueue()) {
        addUnusedRequest(req);
    }
}

void EngineUwsgi::stop()
{
    Q_EMIT enableSockets(false);

    if (thread() != qApp->thread()) {
        thread()->quit();
    }
}

QList<wsgi_request *> EngineUwsgi::unusedRequestQueue() const
{
    return m_unusedReq;
}

void EngineUwsgi::finalizeHeaders(Context *ctx, void *engineData)
{
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(engineData);
    Response *res = ctx->res();

    QByteArray status = statusCode(res->status());
    if (uwsgi_response_prepare_headers(wsgi_req,
                                       status.data(),
                                       status.size())) {
        return;
    }

    // TODO this breads weighttp as it expects keep-alive string
    res->addHeaderValue(m_headerConnectionKey, m_headerConnectionValue);

    QList<HeaderValuePair> headers = res->headers().headersForResponse();
    Q_FOREACH (HeaderValuePair pair, headers) {
        QByteArray &key = pair.key;
        QByteArray &value = pair.value;
        if (uwsgi_response_add_header(wsgi_req,
                                      key.data(),
                                      key.size(),
                                      value.data(),
                                      value.size())) {
            return;
        }
    }
}

bool EngineUwsgi::init()
{
    return true;
}

void EngineUwsgi::forked()
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

    // Start Monitoring Sockets
    struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
    while(uwsgi_sock) {
        watchSocket(uwsgi_sock);
        uwsgi_sock = uwsgi_sock->next;
    }
}
