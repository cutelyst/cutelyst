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
#include "plugin.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

#include <QPluginLoader>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI, "cutelyst.uwsgi")

extern struct uwsgi_server uwsgi;

using namespace Cutelyst;

void cuteOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        uwsgi_log("%s[debug] %s\n", context.category, localMsg.constData());
        break;
    case QtWarningMsg:
        uwsgi_log("%s[warn] %s\n", context.category, localMsg.constData());
        break;
    case QtCriticalMsg:
        uwsgi_log("%s[crit] %s\n", context.category, localMsg.constData());
        break;
    case QtFatalMsg:
        uwsgi_log("%s[fatal] %s\n", context.category, localMsg.constData());
        abort();
    }
}

EngineUwsgi::EngineUwsgi(QObject *parent) :
    Engine(parent)
{
}

bool EngineUwsgi::loadApplication(const QString &path)
{
    if (m_loader) {
        delete m_loader->instance();
        delete m_loader;
    }

    m_loader = new QPluginLoader(path, this);
    if (m_loader->load()) {
        QObject *instance = m_loader->instance();
        if (instance) {
            m_app = qobject_cast<Application *>(instance);
            if (m_app) {
                qCDebug(CUTELYST_UWSGI) << "Application"
                                        << m_app->applicationName()
                                        << "loaded.";

                return true;
            } else {
                qCCritical(CUTELYST_UWSGI) << "Could not create an instance of the application:" << instance;
            }
        } else {
            qCCritical(CUTELYST_UWSGI) << "Could not create an instance:" << path;
        }
    } else {
        qCWarning(CUTELYST_UWSGI) << "Failed to open app:" << m_loader->errorString();
    }
    return false;
}

void EngineUwsgi::finalizeBody(Context *ctx)
{
    Response *res = ctx->res();
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(requestPtr(ctx->req()));

    uwsgi_response_write_body_do(wsgi_req, res->body().data(), res->body().size());
}

void EngineUwsgi::processRequest(struct wsgi_request *wsgi_req)
{
    Request *request;
    QByteArray host = QByteArray::fromRawData(wsgi_req->host, wsgi_req->host_len);
    QByteArray path = QByteArray::fromRawData(wsgi_req->path_info, wsgi_req->path_info_len);
    QUrlQuery queryString(QByteArray::fromRawData(wsgi_req->query_string, wsgi_req->query_string_len));
    request = newRequest(wsgi_req,
                         wsgi_req->https_len ? "http" : "https",
                         host,
                         path,
                         queryString);

    QByteArray remote = QByteArray::fromRawData(wsgi_req->remote_addr, wsgi_req->remote_addr_len);

    QByteArray method = QByteArray::fromRawData(wsgi_req->method, wsgi_req->method_len);

    QByteArray protocol = QByteArray::fromRawData(wsgi_req->protocol, wsgi_req->protocol_len);

    QHash<QByteArray, QByteArray> headers;
    for (int i = 0; i < wsgi_req->var_cnt; i += 2) {
        if (wsgi_req->hvec[i].iov_len < 6) {
            continue;
        }

        if (!uwsgi_startswith((char *) wsgi_req->hvec[i].iov_base,
                              const_cast<char *>("HTTP_"), 5)) {
            QByteArray key = QByteArray::fromRawData((char *) wsgi_req->hvec[i].iov_base+5, wsgi_req->hvec[i].iov_len-5);
            QByteArray value = QByteArray::fromRawData((char *) wsgi_req->hvec[i + 1].iov_base, wsgi_req->hvec[i + 1].iov_len);
            headers.insert(httpCase(key), value);
        }
    }

    QByteArray contentType = QByteArray::fromRawData(wsgi_req->content_type, wsgi_req->content_type_len);
    if (!contentType.isNull()) {
        headers.insert("Content-Type", contentType);
    }

    QByteArray contentEncoding = QByteArray::fromRawData(wsgi_req->encoding, wsgi_req->encoding_len);
    if (!contentEncoding.isNull()) {
        headers.insert("Content-Encoding", contentEncoding);
    }

    QByteArray remoteUser = QByteArray::fromRawData(wsgi_req->remote_user, wsgi_req->remote_user_len);

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
    QByteArray remotePort = QByteArray::fromRawData(remote_port, remote_port_len);

    QFile *upload = new QFile;
    if (wsgi_req->post_file && !upload->open(wsgi_req->post_file, QIODevice::ReadOnly)) {
        qCDebug(CUTELYST_UWSGI) << "Could not open upload file";
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
    struct wsgi_request *wsgi_req = static_cast<wsgi_request*>(requestPtr(ctx->req()));

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

bool EngineUwsgi::postFork()
{
    if (m_app) {
        return setupApplication(m_app);
    } else {
        qCWarning(CUTELYST_UWSGI) << "Trying to setup an not loaded Application";
        return false;
    }
}

EngineUwsgi *engine;

extern "C" int uwsgi_cutelyst_init()
{
    uwsgi_log("Initializing Cutelyst plugin\n");

    engine = new EngineUwsgi;

    return 0;
}

extern "C" void uwsgi_cutelyst_post_fork()
{
    if (!engine->postFork()) {
        qCCritical(CUTELYST_UWSGI) << "Could not setup application on post fork";

#ifdef UWSGI_GO_CHEAP_CODE
        // We need to tell the master process that the
        // application failed to setup and that it shouldn't
        // try to respawn the worker
        exit(UWSGI_GO_CHEAP_CODE);
#endif // UWSGI_GO_CHEAP_CODE
    }
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

// register the new loop engine
extern "C" void uwsgi_cutelyst_on_load() {
    // This allows for some stuff to run event loops
    (void) new QCoreApplication(uwsgi.argc, uwsgi.argv);

    qInstallMessageHandler(cuteOutput);
}

static void fsmon_reload(struct uwsgi_fsmon *fs) {
    qCDebug(CUTELYST_UWSGI) << "Reloading application due to file change";
    uwsgi_reload(uwsgi.argv);
}

extern "C" void uwsgi_cutelyst_init_apps()
{
    uwsgi_log("Cutelyst Init App\n");

    QString path(options.app);
    if (path.isEmpty()) {
        qCCritical(CUTELYST_UWSGI) << "Cytelyst Application was not set";
        return;
    }

    qCDebug(CUTELYST_UWSGI) << "file reload" << options.reload;
    if (options.reload) {

        // Register application reload
        char *file = qstrdup(path.toUtf8().constData());
        uwsgi_register_fsmon(file, fsmon_reload, NULL);
    }

    qCDebug(CUTELYST_UWSGI) << "Loading" << path;
    if (!engine->loadApplication(path)) {
        qCCritical(CUTELYST_UWSGI) << "Could not load application:" << path;
        return;
    }

    // register a new app under a specific "mountpoint"
    uwsgi_add_app(1, CUTELYST_MODIFIER1, NULL, 0, NULL, NULL);
}
