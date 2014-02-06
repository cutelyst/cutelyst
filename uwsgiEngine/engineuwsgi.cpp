#include "engineuwsgi.h"
#include "plugin.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

#include <QPluginLoader>
#include <QFile>
#include <QUrl>
#include <QDebug>

extern struct uwsgi_server uwsgi;

using namespace Cutelyst;

EngineUwsgi::EngineUwsgi(QObject *parent) :
    Engine(parent),
    m_loader(0)
{
}

bool EngineUwsgi::loadApplication(const QString &path)
{
    if (m_loader) {
        delete m_loader->instance();
        delete m_loader;
    }

    m_loader = new QPluginLoader(path);
    if (m_loader->load()) {
        QObject *instance = m_loader->instance();
        if (instance) {
            Application *app = qobject_cast<Application *>(instance);
            if (app) {
                qDebug() << "Application"
                         << app->applicationName()
                         << "loaded.";
                return setupApplication(app);
            } else {
                qCritical() << "Could not create an instance of the application:" << instance;
            }
        } else {
            qCritical() << "Could not create an instance:" << path;
        }
    } else {
        qWarning() << "Failed to open app:" << m_loader->errorString();
    }
    return false;
}

void EngineUwsgi::finalizeBody(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->req()->connectionId());

    uwsgi_response_write_body_do(wsgi_req, res->body().data(), res->body().size());
}

void EngineUwsgi::processRequest(struct wsgi_request *wsgi_req)
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

    QHash<QByteArray, QByteArray> headers;
    for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
        if (wsgi_req->hvec[i].iov_len < 6) {
            continue;
        }

        if (!uwsgi_startswith((char *) wsgi_req->hvec[i].iov_base,
                              const_cast<char *>("HTTP_"), 5)) {
            QByteArray key((char *) wsgi_req->hvec[i].iov_base+5, wsgi_req->hvec[i].iov_len-5);
            QByteArray value((char *) wsgi_req->hvec[i + 1].iov_base, wsgi_req->hvec[i + 1].iov_len);
            headers.insert(httpCase(key), value);
        }
    }

    QByteArray remoteUser(wsgi_req->remote_user, wsgi_req->remote_user_len);

    QByteArray bodyArray;
    size_t remains = wsgi_req->post_cl;
    while(remains > 0) {
        ssize_t body_len = 0;
        char *body =  uwsgi_request_body_read(wsgi_req, UMIN(remains, 32768) , &body_len);
        if (!body || body == uwsgi.empty) {
            break;
        }

        bodyArray.append(body, body_len);
    }

    uint16_t remote_port_len;
    char *remote_port = uwsgi_get_var(wsgi_req, (char *) "REMOTE_PORT", 11, &remote_port_len);
    QByteArray remotePort(remote_port, remote_port_len);

    QFile *upload = new QFile;
    if (wsgi_req->post_file && !upload->open(wsgi_req->post_file, QIODevice::ReadOnly)) {
        qDebug() << "Could not open upload file";
    }

    setupRequest(request,
                 method,
                 protocol,
                 headers,
                 bodyArray,
                 remoteUser,
                 QHostAddress(remote.data()),
                 remotePort.toUInt(),
                 upload);

    handleRequest(request, new Response);
}

QByteArray EngineUwsgi::httpCase(const QByteArray &headerKey) const
{
    QByteArray ret;

    bool lastWasUnderscore = false;

    for (int i = 0 ; i < headerKey.size() ; ++i) {
        QChar buf = headerKey[i];
        if(i == 0 || lastWasUnderscore) {
            ret += buf.toUpper();
            lastWasUnderscore = false;
        } else  if (buf == '_') {
            ret += '-';
            lastWasUnderscore = true;
        } else {
            ret += buf.toLower();
            lastWasUnderscore = false;
        }
    }

    return ret;
}

void EngineUwsgi::finalizeHeaders(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(ctx->req()->connectionId());

    if (uwsgi_response_prepare_headers(wsgi_req,
                                       res->statusCode().data(),
                                       res->statusCode().size())) {
        return;
    }

    QMap<QByteArray, QByteArray> headers = ctx->res()->headers();
    QMap<QByteArray, QByteArray>::Iterator it = headers.begin();
    while (it != headers.end()) {
        QByteArray key = it.key();
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

bool EngineUwsgi::init()
{
    return true;
}

EngineUwsgi *engine;

extern "C" int uwsgi_cutelyst_init()
{
    uwsgi_log("Initializing Cutelyst plugin\n");

    engine = new EngineUwsgi;

    return 0;
}

extern "C" int uwsgi_cutelyst_request(struct wsgi_request *wsgi_req)
{
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

extern "C" void uwsgi_cutelyst_init_apps()
{
    uwsgi_log("Cutelyst Init App\n");

    QString path(options.app);
    if (path.isEmpty()) {
        qCritical() << "Cytelyst Application was not set";
        return;
    }

    qDebug()  << "Loading" << path;
    if (!engine->loadApplication(path)) {
        qCritical() << "Could not load application:" << path;
        return;
    }

    // get the current app id
    int id = uwsgi_apps_cnt;

    // register a new app under a specific "mountpoint"
    uwsgi_add_app(id, CUTELYST_MODIFIER1, NULL, 0, NULL, NULL);
}
