#include "engineuwsgi.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

#include <QPluginLoader>
#include <QUrl>

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

struct wsgi_request *_wsgi_req;

void CutelystEngineUwsgi::finalizeBody(Context *ctx)
{
    QByteArray body = ctx->res()->body();
    qDebug() << "BODY" << body;
    uwsgi_response_prepare_headers(_wsgi_req, (char *)"200 OK", 6);
    uwsgi_response_add_content_type(_wsgi_req, (char *)"text/html", 9);
    uwsgi_response_write_body_do(_wsgi_req, body.data(), body.size());
}

void CutelystEngineUwsgi::processRequest(struct wsgi_request *req)
{
    char *url;
    uint16_t url_len;
    url = uwsgi_get_var(req, (char *) "PATH_INFO", 9, &url_len);

    QUrl urlParsed = QString(url).trimmed();
    qDebug() << url << urlParsed;
    createRequest(0,
                  urlParsed,
                  "GET",
                  QString(),
                  QHash<QString, QByteArray>(),
                  QByteArray());
}

void CutelystEngineUwsgi::finalizeHeaders(Context *ctx)
{

}

bool CutelystEngineUwsgi::init()
{

}

CutelystEngineUwsgi *engine;

class FakeClass {

public:
    char *foobar;
    uint16_t foobar_len;
    void hello_world(struct wsgi_request *);

};

void FakeClass::hello_world(struct wsgi_request *wsgi_req) {

    uwsgi_response_prepare_headers(wsgi_req, (char *)"200 OK", 6);
    uwsgi_response_add_content_type(wsgi_req, (char *)"text/html", 9);
    uwsgi_response_write_body_do(wsgi_req, foobar, foobar_len);
}

extern "C" int uwsgi_cplusplus_init(){
    uwsgi_log("Initializing Cutelyst C++ plugin\n");
    engine = new CutelystEngineUwsgi("/home/daniel/code/iglooshop/build/srv/libcuteserver.so");
    //        uwsgi_add_app()
    return 0;
}

extern "C" int uwsgi_cplusplus_request(struct wsgi_request *wsgi_req) {


    uwsgi_log("New request.\n");

    FakeClass *fc;

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

    _wsgi_req = wsgi_req;
    engine->processRequest(wsgi_req);
    //        fc = new FakeClass();
    //        // get PATH_INFO
    //        fc->foobar = uwsgi_get_var(wsgi_req, (char *) "PATH_INFO", 9, &fc->foobar_len);

    //        if (fc->foobar) {
    //                // send output
    //                fc->hello_world(wsgi_req);
    //        }

    //        delete fc;

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
