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

#include <QBuffer>

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request.h>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI, "cutelyst.uwsgi")

using namespace Cutelyst;

EngineUwsgi::EngineUwsgi(QObject *parent) :
    Engine(parent)
{
}

EngineUwsgi::~EngineUwsgi()
{
    delete m_app;
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
                return initApplication(m_app, false);
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

void EngineUwsgi::processRequest(wsgi_request *req)
{
    Request *request;
    QByteArray host = QByteArray::fromRawData(req->host, req->host_len);
    QByteArray path = QByteArray::fromRawData(req->path_info, req->path_info_len);
    QUrlQuery queryString(QByteArray::fromRawData(req->query_string, req->query_string_len));
    request = newRequest(req,
                         req->https_len ? "http" : "https",
                         host,
                         path,
                         queryString);

    QHostAddress remoteAddress(QByteArray::fromRawData(req->remote_addr, req->remote_addr_len).data());

    QByteArray method = QByteArray::fromRawData(req->method, req->method_len);

    QByteArray protocol = QByteArray::fromRawData(req->protocol, req->protocol_len);

    QHash<QByteArray, QByteArray> headers;
    for (int i = 0; i < req->var_cnt; i += 2) {
        if (req->hvec[i].iov_len < 6) {
            continue;
        }

        if (!uwsgi_startswith((char *) req->hvec[i].iov_base,
                              const_cast<char *>("HTTP_"), 5)) {
            QByteArray key = QByteArray::fromRawData((char *) req->hvec[i].iov_base+5, req->hvec[i].iov_len-5);
            QByteArray value = QByteArray::fromRawData((char *) req->hvec[i + 1].iov_base, req->hvec[i + 1].iov_len);
            headers.insert(httpCase(key), value);
        }
    }

    QByteArray contentType = QByteArray::fromRawData(req->content_type, req->content_type_len);
    if (!contentType.isNull()) {
        headers.insert("Content-Type", contentType);
    }

    QByteArray contentEncoding = QByteArray::fromRawData(req->encoding, req->encoding_len);
    if (!contentEncoding.isNull()) {
        headers.insert("Content-Encoding", contentEncoding);
    }

    QByteArray remoteUser = QByteArray::fromRawData(req->remote_user, req->remote_user_len);

    uint16_t remote_port_len;
    char *remote_port = uwsgi_get_var(req, (char *) "REMOTE_PORT", 11, &remote_port_len);
    QByteArray remotePort = QByteArray::fromRawData(remote_port, remote_port_len);

    QFile *upload = new QFile;
    if (req->post_file && !upload->open(req->post_file, QIODevice::ReadOnly)) {
        qCDebug(CUTELYST_UWSGI) << "Could not open upload file";
    }

    QByteArray bodyArray;
    size_t remains = req->post_cl;
    while(remains > 0) {
        ssize_t body_len = 0;
        char *body =  uwsgi_request_body_read(req, UMIN(remains, 32768) , &body_len);
        if (!body || body == uwsgi.empty) {
            break;
        }

        bodyArray.append(body, body_len);
    }

    QBuffer *buffer = new QBuffer(&bodyArray);
    buffer->open(QBuffer::ReadOnly);
    setupRequest(request,
                 method,
                 protocol,
                 headers,
                 buffer,
                 remoteUser,
                 remoteAddress,
                 remotePort.toUInt());

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

void EngineUwsgi::reload()
{
    qCDebug(CUTELYST_UWSGI) << "Reloading application due application request";
    uwsgi_reload(uwsgi.argv);
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
    return postForkApplication();
}
