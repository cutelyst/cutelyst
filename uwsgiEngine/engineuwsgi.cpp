/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QtCore/QSocketNotifier>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QFile>

#include <Cutelyst/common.h>
#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI, "cutelyst.uwsgi")

using namespace Cutelyst;

uWSGI::uWSGI(Application *app, int workerCore, const QVariantMap &opts) : Engine(app, workerCore, opts)
{
    connect(this, &uWSGI::postFork, this, &uWSGI::forked);
}

uWSGI::~uWSGI()
{
}

int uWSGI::workerId() const
{
    return m_workerId;
}

void uWSGI::setWorkerId(int id)
{
    m_workerId = id;
}

void uWSGI::setThread(QThread *thread)
{
    moveToThread(thread);
    if (this->thread() == thread) {
        connect(thread, &QThread::started,
                this, &uWSGI::forked, Qt::DirectConnection);
    } else {
        qCWarning(CUTELYST_UWSGI) << "Failed to set thread";
    }
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
    Q_FOREVER {
        int ret = uwsgi_wait_read_req(wsgi_req);
        if (ret <= 0) {
            qCDebug(CUTELYST_UWSGI) << "Failed wait read.";
            return;
        }

        int status = wsgi_req->socket->proto(wsgi_req);
        if (status < 0) {
            qCDebug(CUTELYST_UWSGI) << "Failed broken socket.";
            return;
        } else if (status == 0) {
            break;
        }
    }

    validateAndExecuteRequest(wsgi_req, 0);
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
    // wsgi_req->uri containg the whole URI it /foo/bar?query=null
    // so we use path_info, maybe it would be better to just build our
    // Request->uri() from it, but we need to run a performance test
    EngineRequest request;

    uint16_t pos = notSlash(req->path_info, req->path_info_len);
    request.path = QString::fromLatin1(req->path_info + pos, req->path_info_len - pos);

    request.serverAddress = QString::fromLatin1(req->host, req->host_len);
    request.query = QByteArray::fromRawData(req->query_string, req->query_string_len);

    request.method = QString::fromLatin1(req->method, req->method_len);
    request.protocol = QString::fromLatin1(req->protocol, req->protocol_len);
    request.remoteAddress = QHostAddress(QString::fromLatin1(req->remote_addr, req->remote_addr_len));
    request.remoteUser = QString::fromLatin1(req->remote_user, req->remote_user_len);
    request.isSecure = req->https_len;
    request.startOfRequest = req->start_of_request;
    request.requestPtr = req;

    request.remotePort = 0;
    Headers &headers = request.headers;
    // we scan the table in reverse, as updated values are at the end
    for (int i = req->var_cnt - 1; i > 0; i -= 2) {
        struct iovec &name = req->hvec[i - 1];
        struct iovec &value = req->hvec[i];
        if (!uwsgi_startswith(static_cast<char *>(name.iov_base),
                              const_cast<char *>("HTTP_"), 5)) {
            headers.pushRawHeader(QString::fromLatin1(static_cast<char *>(name.iov_base) + 5, name.iov_len - 5),
                                  QString::fromLatin1(static_cast<char *>(value.iov_base), value.iov_len));
        } else if (!request.remotePort &&
                   !uwsgi_strncmp(const_cast<char *>("REMOTE_PORT"), 11,
                                  static_cast<char *>(name.iov_base), name.iov_len)) {
            request.remotePort = QByteArray::fromRawData(static_cast<char *>(value.iov_base), value.iov_len).toUInt();
        }
    }

    if (req->content_type_len > 0) {
        headers.setContentType(QString::fromLatin1(req->content_type, req->content_type_len));
    }

    if (req->encoding_len > 0) {
        headers.setContentEncoding(QString::fromLatin1(req->encoding, req->encoding_len));
    }

    QIODevice *body = nullptr;
    if (req->post_cl) {
        if (req->post_file) {
            //        qCDebug(CUTELYST_UWSGI) << "Post file available:" << req->post_file;
            auto upload = new QFile;
            if (upload->open(req->post_file, QIODevice::ReadOnly)) {
                body = upload;
            } else {
                //            qCDebug(CUTELYST_UWSGI) << "Could not open post file:" << upload->errorString();
                body = new BodyUWSGI(req, !uwsgi.post_buffering);
                body->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
            }
        } else {
            //        qCDebug(CUTELYST_UWSGI) << "Post buffering size:" << uwsgi.post_buffering;
            body = new BodyUWSGI(req, !uwsgi.post_buffering);
            body->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        }
    }
    request.body = body;

    Engine::processRequest(request);

    delete body;
}

void uWSGI::addUnusedRequest(wsgi_request *wsgi_req)
{
    m_unusedReq.push_back(wsgi_req);
}

uwsgi_socket* uWSGI::watchSocket(struct uwsgi_socket *uwsgi_sock)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(uwsgi_sock->fd, QSocketNotifier::Read, this);
    connect(this, &uWSGI::enableSockets,
            socketNotifier, &QSocketNotifier::setEnabled);
    connect(socketNotifier, &QSocketNotifier::activated,
            [=](int fd) {
        if (m_unusedReq.empty()) {
            uwsgi_async_queue_is_full(uwsgi_now());
            return;
        }

        struct wsgi_request *wsgi_req = m_unusedReq.back();

        // fill wsgi_request structure
        wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock);

        // mark core as used
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;

        // accept the connection
        if (wsgi_req_simple_accept(wsgi_req, fd) != 0) {
            uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
            m_unusedReq.push_back(wsgi_req);
            return;
        }
        m_unusedReq.pop_back();

        wsgi_req->start_of_request = uwsgi_micros();
        wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

#ifdef UWSGI_GO_CHEAP_CODE
        // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
            set_harakiri(uwsgi.harakiri_options.workers);
        }
#endif // UWSGI_GO_CHEAP_CODE

        readRequestUWSGI(wsgi_req);
        uwsgi_close_request(wsgi_req);

        m_unusedReq.push_back(wsgi_req);
    });

    return uwsgi_sock->next;
}

void uWSGI::validateAndExecuteRequest(wsgi_request *wsgi_req, int status)
{
    if (status < 0) {
        qCDebug(CUTELYST_UWSGI) << "Failed broken socket.";
        return;
    }

    // empty request ?
    if (!wsgi_req->uh->pktsize) {
        qCDebug(CUTELYST_UWSGI) << "Empty request. skip.";
        return;
    }

    // get uwsgi variables
    if (uwsgi_parse_vars(wsgi_req)) {
        // If static maps are set or there is some error
        // this returns -1 so we just close the request
        return;
    }

    processRequest(wsgi_req);
}

uwsgi_socket *uWSGI::watchSocketAsync(struct uwsgi_socket *uwsgi_sock)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(uwsgi_sock->fd, QSocketNotifier::Read, this);
    connect(this, &uWSGI::enableSockets,
            socketNotifier, &QSocketNotifier::setEnabled);
    connect(socketNotifier, &QSocketNotifier::activated,
            [=](int fd) {
        if (m_unusedReq.empty()) {
            uwsgi_async_queue_is_full(uwsgi_now());
            return;
        }

        struct wsgi_request *wsgi_req = m_unusedReq.back();

        // fill wsgi_request structure
        wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock);

        // mark core as used
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;

        // accept the connection (since uWSGI 1.5 all of the sockets are non-blocking)
        if (wsgi_req_simple_accept(wsgi_req, fd) != 0) {
            uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
            m_unusedReq.push_back(wsgi_req);
            return;
        }
        m_unusedReq.pop_back();

        wsgi_req->start_of_request = uwsgi_micros();
        wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

#ifdef UWSGI_GO_CHEAP_CODE
        // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
            set_harakiri(uwsgi.harakiri_options.workers);
        }
#endif // UWSGI_GO_CHEAP_CODE

        QSocketNotifier *requestNotifier = new QSocketNotifier(wsgi_req->fd, QSocketNotifier::Read, this);
        QTimer *timeoutTimer = new QTimer(requestNotifier);

#ifdef UWSGI_GO_CHEAP_CODE
        timeoutTimer->setInterval(uwsgi.socket_timeout * 1000);
#else
        timeoutTimer->setInterval(15 * 1000);
#endif // UWSGI_GO_CHEAP_CODE

        connect(timeoutTimer, &QTimer::timeout,
                [=]() {
            requestNotifier->setEnabled(false);
            uwsgi_close_request(wsgi_req);
            requestNotifier->deleteLater();
            m_unusedReq.push_back(wsgi_req);
        });

        connect(requestNotifier, &QSocketNotifier::activated,
                [=]() {
            int status = wsgi_req->socket->proto(wsgi_req);
            if (status > 0) {
                // still need to read
                timeoutTimer->start();
                return;
            }

            // Disable the notifier because we are in async
            // mode and more data would break things
            requestNotifier->setEnabled(false);
            delete timeoutTimer;

            validateAndExecuteRequest(wsgi_req, status);

            uwsgi_close_request(wsgi_req);
            m_unusedReq.push_back(wsgi_req);

            requestNotifier->deleteLater();
        });

        timeoutTimer->start();

    });

    return uwsgi_sock->next;
}

void uWSGI::stop()
{
    Q_EMIT enableSockets(false);

    if (thread() != qApp->thread()) {
        thread()->quit();
    }
}

quint64 uWSGI::time()
{
    return uwsgi_micros();
}

bool uWSGI::finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData)
{
    auto wsgi_req = static_cast<wsgi_request*>(engineData);

    if (uwsgi_response_prepare_headers_int(wsgi_req, status)) {
        return false;
    }

    const auto headersData = headers.data();
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
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
    return initApplication();
}

bool uWSGI::forked()
{
    if (workerCore() > 0) {
        // init and postfork
        if (!initApplication()) {
            qFatal("Failed to init application on a different thread than main. Are you sure threaded mode is supported in this application?");
            return false;
        }
    }

    if (!postForkApplication()) {
#ifdef UWSGI_GO_CHEAP_CODE
        // We need to tell the master process that the
        // application failed to setup and that it shouldn't
        // try to respawn the worker
        exit(UWSGI_GO_CHEAP_CODE);
#endif // UWSGI_GO_CHEAP_CODE
    }

    if (!m_unusedReq.empty()) {
        // Start Monitoring Sockets
        struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
        if (m_unusedReq.size() > 1) {
            while(uwsgi_sock) {
                uwsgi_sock = watchSocketAsync(uwsgi_sock);
            }
        } else {
            while(uwsgi_sock) {
                uwsgi_sock = watchSocket(uwsgi_sock);
            }
        }
    }

    return true;
}

#include "moc_engineuwsgi.cpp"
