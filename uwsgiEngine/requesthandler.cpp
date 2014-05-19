#include "requesthandler.h"
//#include "engineuwsgi.h"

#include <QDebug>

#undef signals

#include <uwsgi.h>

extern struct uwsgi_server uwsgi;

using namespace Cutelyst;

RequestHandler::RequestHandler(wsgi_request *request)
    : wsgi_req(request)
{
}

// manage signal handlers
void RequestHandler::handle_signal(int fd) {
    qDebug() << Q_FUNC_INFO << fd;
    uwsgi_receive_signal(fd, (char *) "worker", uwsgi.mywid);
}

// manage requests
void RequestHandler::handle_request(int fd) {
    struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
        while(uwsgi_sock) {
                if (uwsgi_sock->fd == fd) break;
                uwsgi_sock = uwsgi_sock->next;
        }

    if (!uwsgi_sock) return;

    wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock );
    // mark core as used
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;
    if (wsgi_req_simple_accept(wsgi_req, uwsgi_sock->fd)) {
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
        return;
    }

    wsgi_req->start_of_request = uwsgi_micros();
        wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

    // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
                set_harakiri(uwsgi.harakiri_options.workers);
        }

    for(;;) {
                int ret = uwsgi.wait_read_hook(wsgi_req->fd, uwsgi.socket_timeout);
                wsgi_req->switches++;

                if (ret <= 0) {
                        goto end;
                }

                int status = wsgi_req->socket->proto(wsgi_req);
                if (status < 0) {
                        goto end;
                }
                else if (status == 0) {
                        break;
                }
        }

#ifdef UWSGI_ROUTING
        if (uwsgi_apply_routes(wsgi_req) == UWSGI_ROUTE_BREAK) {
                goto end;
        }
#endif

        for(;;) {
                if (uwsgi.p[wsgi_req->uh->modifier1]->request(wsgi_req) <= UWSGI_OK) {
                        goto end;
                }
                wsgi_req->switches++;
    }

end:
    uwsgi_close_request(wsgi_req);
    return;
}
