#include "engineuwsgi.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

#include <QPluginLoader>
#include <QUrl>
#include <QDebug>
#include <QStringBuilder>

extern struct uwsgi_server uwsgi;

using namespace Cutelyst;

CutelystEngineUwsgi::CutelystEngineUwsgi(Application *parent) :
    Engine(parent)
{
}

void CutelystEngineUwsgi::finalizeBody(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->req()->connectionId());

    uwsgi_response_write_body_do(wsgi_req, res->body().data(), res->body().size());
}

void CutelystEngineUwsgi::processRequest(struct wsgi_request *wsgi_req)
{
    Request *request;
    QByteArray host(wsgi_req->host, wsgi_req->host_len);
    QByteArray path(wsgi_req->path_info, wsgi_req->path_info_len);
    QUrlQuery queryString(QByteArray(wsgi_req->query_string, wsgi_req->query_string_len));
    request = newRequest(wsgi_req,
                         wsgi_req->https_len ? "http" : "https",
                         host,
                         path,
                         queryString);

    QByteArray remote(wsgi_req->remote_addr, wsgi_req->remote_addr_len);

    QByteArray method(wsgi_req->method, wsgi_req->method_len);

    QByteArray protocol(wsgi_req->protocol, wsgi_req->protocol_len);

    size_t remains = wsgi_req->post_pos;
    qDebug() << "remains" << remains << "query string" << QByteArray(wsgi_req->query_string, wsgi_req->query_string_len);
    qDebug() << "document_root" << QByteArray(wsgi_req->document_root, wsgi_req->document_root_len);
    qDebug() << "cookie" << QByteArray(wsgi_req->cookie, wsgi_req->cookie_len);
    qDebug() << "uri" << QByteArray(wsgi_req->uri, wsgi_req->uri_len);
    qDebug() << "path_info" << QByteArray(wsgi_req->path_info, wsgi_req->path_info_len);
    qDebug() << "host" << QByteArray(wsgi_req->host, wsgi_req->host_len);
    qDebug() << "remote_addr" << QByteArray(wsgi_req->remote_addr, wsgi_req->remote_addr_len);
    qDebug() << "https" << QByteArray(wsgi_req->https, wsgi_req->https_len);
    qDebug() << "content_type" << QByteArray(wsgi_req->content_type, wsgi_req->content_type_len);
    qDebug() << "post_pos" << wsgi_req->post_pos;
    qDebug() << "header_cnt" << wsgi_req->header_cnt;
    qDebug() << "var_cnt" << wsgi_req->var_cnt;
    qDebug() << "headers_size" << wsgi_req->headers_size;
//    qDebug() << "header" << QByteArray(req->headers->buf);


//    for (size_t i = 0; i < req->headers->len; ++i) {
//        qDebug() << "header" << i << QByteArray(req->headers->buf);
//    }

    ssize_t body_len = 0;
    char *body_char =  uwsgi_request_body_read(wsgi_req, UMIN(remains, 32768) , &body_len);
    QByteArray body(body_char, body_len);

    QByteArray cookies(wsgi_req->cookie, wsgi_req->cookie_len);

    QHash<QByteArray, QByteArray> headers;
    headers.insert("Content-Type", QByteArray(wsgi_req->content_type, wsgi_req->content_type_len));
    headers.insert("Content-Length", QByteArray::number(body.size()));
    headers.insert("User-Agent", QByteArray(wsgi_req->user_agent, wsgi_req->user_agent_len));
    headers.insert("Cookie", cookies);

    setupRequest(request,
                 method,
                 protocol,
                 headers,
                 body,
                 QHostAddress(remote.data()));

    qDebug() << method << remote << path << protocol;
    qDebug() << body;

    qDebug() << "---> URI" << request->uri();
    qDebug() << "---> base" << request->base();
    qDebug() << "---> path" << request->path();
    qDebug() << "---> peerAddress" << request->peerAddress();
    qDebug() << "---> queryParam" << request->queryParam();

    handleRequest(request, new Response);
}

void CutelystEngineUwsgi::finalizeHeaders(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->req()->connectionId());

    if (uwsgi_response_prepare_headers(wsgi_req,
                                       res->statusCode().data(),
                                       res->statusCode().size())) {
        return;
    }

    QMap<QByteArray, QByteArray> headers = ctx->res()->headers();
    if (uwsgi_response_add_content_type(wsgi_req,
                                        res->contentType().data(),
                                        res->contentType().size())) {
        return;
    }
    headers.remove("Content-Type");

    QMap<QByteArray, QByteArray>::Iterator it = headers.begin();
    while (it != headers.end()) {
        QByteArray key = it.key();
        qCritical() << key << it.value();
        if (uwsgi_response_add_header(wsgi_req,
                                      key.data(),
                                      key.size(),
                                      it.value().data(),
                                      it.value().size())) {
            return;
        }
        ++it;
    }
}

bool CutelystEngineUwsgi::init()
{

}

CutelystEngineUwsgi *engine;

extern "C" int uwsgi_cplusplus_init(){
    uwsgi_log("Initializing Cutelyst C++ plugin\n");

    QPluginLoader *loader = new QPluginLoader("/home/daniel/code/iglooshop/build/srv/libcuteserver.so");
    if (loader->load()) {
        QObject *instance = loader->instance();
        if (instance) {
            Application *app = qobject_cast<Application *>(instance);
            if (app) {
                qDebug() << "Application"
                         << app->applicationName()
                         << "loaded.";
                engine = new CutelystEngineUwsgi(app);
                app->setup(engine);
            } else {
                qCritical() << "Could not create an instance of the application:" << instance;
            }
        }
    } else {
        qWarning() << "Failed to open app:" << loader->errorString();
    }
    delete loader;
    //        uwsgi_add_app()
    return 0;
}

extern "C" int uwsgi_cplusplus_request(struct wsgi_request *wsgi_req)
{
    uwsgi_log("New request.\n");

    // empty request ?
    if (!wsgi_req->uh->pktsize) {
        uwsgi_log( "Invalid request. skip.\n");
        goto clear;
    }

    // get uwsgi variables
    if (uwsgi_parse_vars(wsgi_req)) {
        uwsgi_log("Invalid request. skip.\n");
        goto clear;
    }

    engine->processRequest(wsgi_req);

clear:
    return UWSGI_OK;
}

extern "C" void uwsgi_cplusplus_after_request(struct wsgi_request *wsgi_req) {
    // call log_request(wsgi_req) if you want a standard logline
    uwsgi_log("logging c++ request\n");
}

extern "C" void uwsgi_cutelyst_init_apps()
{
    uwsgi_log("CUTELYST INIT APPS\n");
}
