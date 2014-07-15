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

#include "requesthandler.h"
#include "engineuwsgi.h"

#include <QDebug>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(CUTELYST_UWSGI_QTLOOP, "cutelyst.uwsgi.qtloop")

#define free_req_queue uwsgi.async_queue_unused_ptr++; uwsgi.async_queue_unused[uwsgi.async_queue_unused_ptr] = wsgi_req

RequestHandler::RequestHandler()
{
}

// manage requests
void RequestHandler::handle_request(int fd)
{
//    qCDebug(CUTELYST_UWSGI_QTLOOP) << thread() << fd;
    struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
    while(uwsgi_sock) {
        if (uwsgi_sock->fd == fd) {
            break;
        }
        uwsgi_sock = uwsgi_sock->next;
    }

    if (!uwsgi_sock) {
        return;
    }

    struct wsgi_request *wsgi_req = find_first_available_wsgi_req();
    if (wsgi_req == NULL) {
        uwsgi_async_queue_is_full(uwsgi_now());
        return;
    }

    // fill wsgi_request structure
    wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock);

    qCDebug(CUTELYST_UWSGI_QTLOOP) << "wsgi_req->async_id" << wsgi_req->async_id << fd;
    qCDebug(CUTELYST_UWSGI_QTLOOP) << "in_request" << uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request;

    // mark core as used
    uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;

    // accept the connection
    if (wsgi_req_simple_accept(wsgi_req, uwsgi_sock->fd)) {
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
        free_req_queue;
        return;
    }

    wsgi_req->start_of_request = uwsgi_micros();
    wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

    // enter harakiri mode
    if (uwsgi.harakiri_options.workers > 0) {
        set_harakiri(uwsgi.harakiri_options.workers);
    }
    qCDebug(CUTELYST_UWSGI_QTLOOP) << "in_request" << uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request;


    for(;;) {
        int ret = uwsgi_wait_read_req(wsgi_req);

        if (ret <= 0) {
            goto end;
        }

        int status = wsgi_req->socket->proto(wsgi_req);
        if (status < 0) {
            goto end;
        } else if (status == 0) {
            break;
        }
    }

    qCDebug(CUTELYST_UWSGI_QTLOOP) << "async_environ" << wsgi_req->async_environ;

    for(;;) {
        if (uwsgi.p[wsgi_req->uh->modifier1]->request(wsgi_req) <= UWSGI_OK) {
            goto end;
        }
        wsgi_req->switches++;
    }

end:
    uwsgi_close_request(wsgi_req);
    free_req_queue;
    return;
}
