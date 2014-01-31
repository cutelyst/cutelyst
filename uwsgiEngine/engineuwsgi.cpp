#include "engineuwsgi.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

#include <QPluginLoader>
#include <QUrl>
#include <QHostInfo>
#include <QStringBuilder>

extern struct uwsgi_server uwsgi;

using namespace Cutelyst;

CutelystEngineUwsgi::CutelystEngineUwsgi(const QString &app, QObject *parent) :
    Engine(parent),
    m_app(0)
{
    m_loader = new QPluginLoader(app, this);
    if (m_loader->load()) {
        QObject *instance = m_loader->instance();
        if (instance) {
            m_app = qobject_cast<Application *>(instance);
            if (m_app) {
                m_app->setup(this);
            } else {
                qCritical() << "Could not create an instance of the application:" << instance;
            }
        }
    } else {
        qWarning() << "Failed to open app:" << m_loader->errorString();
    }
}

void CutelystEngineUwsgi::finalizeBody(Context *ctx)
{
    Response *res = ctx->res();

    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->req()->connectionId());

    if (uwsgi_response_prepare_headers(wsgi_req,
                                       res->statusCode().data(),
                                       res->statusCode().size())) {
        return;
    }

    if (uwsgi_response_add_content_type(wsgi_req,
                                        res->contentType().data(),
                                        res->contentType().size())) {
        return;
    }

    uwsgi_response_write_body_do(wsgi_req, res->body().data(), res->body().size());
}

void CutelystEngineUwsgi::processRequest(struct wsgi_request *req)
{
    QString path = QByteArray(req->path_info, req->path_info_len);
    QByteArray remote(req->remote_addr, req->remote_addr_len);
    QByteArray method(req->method, req->method_len);
    QByteArray protocol(req->protocol, req->protocol_len);
    QByteArray queryString(req->query_string, req->query_string_len);

    QByteArray body;
    size_t remains = req->post_cl;
    qDebug() << "remains" << remains << "query string" << queryString;
    while(remains > 0) {
        qDebug() << "remains1" << remains;
        ssize_t body_len = 0;
        char *body_part =  uwsgi_request_body_read(req, UMIN(remains, 32768) , &body_len);
        qDebug() << "remains2" << body_part;
        if (!body_part || body_part == uwsgi.empty) {
            break;
        }
        body.append(body_part, body_len);
    }

    QHash<QByteArray, QByteArray> headers;

    QUrl url;
    if (!remote.isEmpty()) {
        // TODO I think SERVER_NAME:SERVER_PORT is what we want here
        url = QLatin1String("http://") % remote % path;
    } else {
        // This is a hack just in case remote is not set
        url = QLatin1String("http://") % QHostInfo::localHostName() % path;
    }

    qDebug() << url << method << remote << path << protocol;
    qDebug() << body;
    createRequest(req,
                  url,
                  method,
                  protocol,
                  headers,
                  body);
}

void CutelystEngineUwsgi::finalizeHeaders(Context *ctx)
{

}

bool CutelystEngineUwsgi::init()
{

}

CutelystEngineUwsgi *engine;

extern "C" int uwsgi_cplusplus_init(){
    uwsgi_log("Initializing Cutelyst C++ plugin\n");
    engine = new CutelystEngineUwsgi("/home/daniel/code/iglooshop/build/srv/libcuteserver.so");
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
