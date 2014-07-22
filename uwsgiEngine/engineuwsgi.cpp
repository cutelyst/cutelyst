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

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI, "cutelyst.uwsgi")

using namespace Cutelyst;

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

void EngineUwsgi::finalizeBody(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(requestPtr(ctx->req()));

    uwsgi_response_write_body_do(wsgi_req, res->body().data(), res->body().size());
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

//    qCDebug(CUTELYST_UWSGI) << "async_environ" << wsgi_req->async_environ;
    processRequest(wsgi_req);

end:
    uwsgi_close_request(wsgi_req);
}

void EngineUwsgi::processRequest(wsgi_request *req)
{
    Request *request;
    QByteArray host = QByteArray::fromRawData(req->host, req->host_len);
    QByteArray path = QByteArray::fromRawData(req->uri, req->uri_len);
    QUrlQuery queryString(QByteArray::fromRawData(req->query_string, req->query_string_len));
    request = newRequest(req,
                         req->https_len ? "http" : "https",
                         host,
                         path,
                         queryString);

    QHostAddress remoteAddress(QByteArray::fromRawData(req->remote_addr, req->remote_addr_len).data());

    QByteArray method = QByteArray::fromRawData(req->method, req->method_len);

    QByteArray protocol = QByteArray::fromRawData(req->protocol, req->protocol_len);

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

    QByteArray remoteUser = QByteArray::fromRawData(req->remote_user, req->remote_user_len);

    uint16_t remote_port_len;
    char *remote_port = uwsgi_get_var(req, (char *) "REMOTE_PORT", 11, &remote_port_len);
    QByteArray remotePort = QByteArray::fromRawData(remote_port, remote_port_len);

    QIODevice *body;
    if (req->post_file) {
        qCDebug(CUTELYST_UWSGI) << "Post file available:" << req->post_file;
        QFile *upload = new QFile;
        if (upload->open(req->post_file, QIODevice::ReadOnly)) {
            body = upload;
        } else {
//            qCDebug(CUTELYST_UWSGI) << "Could not open post file:" << upload->errorString();
            body = new BodyBufferedUWSGI(req);
        }
    } else if (uwsgi.post_buffering) {
//        qCDebug(CUTELYST_UWSGI) << "Post buffering size:" << uwsgi.post_buffering;
        body = new BodyUWSGI(req);
    } else {
        // BodyBufferedUWSGI is an IO device which will
        // only consume the body when some of it's functions
        // is called, this is because here we can't seek
        // the body.
        body = new BodyBufferedUWSGI(req);
    }

    setupRequest(request,
                 method,
                 protocol,
                 headers,
                 body,
                 remoteUser,
                 remoteAddress,
                 remotePort.toUInt());

    handleRequest(request);
}

QByteArray EngineUwsgi::httpCase(char *key, int key_len) const
{
    bool lastWasUnderscore = false;
    for (int i = 0 ; i < key_len ; ++i) {
        QChar buf = key[i];
        if(!lastWasUnderscore) {
            key[i] = buf.toLower().toLatin1();
            lastWasUnderscore = false;
        } else  if (buf == '_') {
            key[i] = '-';
            lastWasUnderscore = true;
        } else {
            lastWasUnderscore = false;
        }
    }

    return QByteArray::fromRawData(key, key_len);
}

void EngineUwsgi::reload()
{
    qCDebug(CUTELYST_UWSGI) << "Reloading application due application request";
    uwsgi_reload(uwsgi.argv);
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

        // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
            set_harakiri(uwsgi.harakiri_options.workers);
        }

        readRequestUWSGI(wsgi_req);

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

void EngineUwsgi::finalizeHeaders(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(requestPtr(ctx->req()));

    QByteArray status = statusCode(res->status());
    if (uwsgi_response_prepare_headers(wsgi_req,
                                       status.data(),
                                       status.size())) {
        return;
    }

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
    qCDebug(CUTELYST_UWSGI) << "Post-Fork thread id" << QThread::currentThread() << qApp->thread() << thread();

    if (QThread::currentThread() != qApp->thread()) {
        qCDebug(CUTELYST_UWSGI) << "Post-Fork different thread";
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
