/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "uwsgiconnection.h"

#include "engineuwsgi.h"
#include "bodyuwsgi.h"

#include <uwsgi.h>

#include <Cutelyst/Context>

#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_UWSGI_CONN, "cutelyst.uwsgi.connection", QtWarningMsg)

using namespace Cutelyst;

static inline quint16 notSlash(char *str, quint16 length) {
    for (quint16 i = 0; i < length; ++i) {
        if (str[i] != '/') {
            return i;
        }
    }
    return length;
}

static inline quint16 questionMark(char *str, quint16 length) {
    for (quint16 i = 0; i < length; ++i) {
        if (str[i] == '?') {
            return i;
        }
    }
    return length;
}

uwsgiConnection::uwsgiConnection(wsgi_request *req)
  : request(req)
{
    quint16 len = questionMark(req->uri, req->uri_len);
    quint16 pos = notSlash(req->uri, len);
    QByteArray rawPath(req->uri + pos, len - pos);
    setPath(rawPath.data(), rawPath.size());

    serverAddress = QString::fromLatin1(req->host, req->host_len);
    query = QByteArray::fromRawData(req->query_string, req->query_string_len);

    method = QString::fromLatin1(req->method, req->method_len);
    protocol = QString::fromLatin1(req->protocol, req->protocol_len);
    remoteAddress.setAddress(QString::fromLatin1(req->remote_addr, req->remote_addr_len));
    remoteUser = QString::fromLatin1(req->remote_user, req->remote_user_len);
    isSecure = req->https_len;
    startOfRequest = req->start_of_request;
    elapsed.start();

    remotePort = 0;
    // we scan the table in reverse, as updated values are at the end
    for (int i = req->var_cnt - 1; i > 0; i -= 2) {
        struct iovec &name = req->hvec[i - 1];
        struct iovec &value = req->hvec[i];
        if (!uwsgi_startswith(static_cast<char *>(name.iov_base),
                              const_cast<char *>("HTTP_"), 5)) {
            headers.pushRawHeader(QString::fromLatin1(static_cast<char *>(name.iov_base) + 5, name.iov_len - 5),
                                  QString::fromLatin1(static_cast<char *>(value.iov_base), value.iov_len));
        } else if (!remotePort &&
                   !uwsgi_strncmp(const_cast<char *>("REMOTE_PORT"), 11,
                                  static_cast<char *>(name.iov_base), name.iov_len)) {
            remotePort = QByteArray::fromRawData(static_cast<char *>(value.iov_base), value.iov_len).toUInt();
        }
    }

    if (req->content_type_len > 0) {
        headers.setContentType(QString::fromLatin1(req->content_type, req->content_type_len));
    }

    if (req->encoding_len > 0) {
        headers.setContentEncoding(QString::fromLatin1(req->encoding, req->encoding_len));
    }

    body = nullptr;
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
        headers.setContentLength(req->post_cl);
    }
}

uwsgiConnection::~uwsgiConnection()
{
}

bool uwsgiConnection::writeHeaders(quint16 status, const Headers &headers)
{
    if (uwsgi_response_prepare_headers_int(request, status)) {
        return false;
    }

    const auto headersData = headers.data();
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
        const QByteArray key = uWSGI::camelCaseHeader(it.key()).toLatin1();
        const QByteArray value = it.value().toLatin1();

        if (uwsgi_response_add_header(request,
                                      const_cast<char*>(key.constData()),
                                      key.size(),
                                      const_cast<char*>(value.constData()),
                                      value.size())) {
            return false;
        }

        ++it;
    }

    return true;
}

qint64 uwsgiConnection::doWrite(const char *data, qint64 len)
{
    if (uwsgi_response_write_body_do(request,
                                     const_cast<char *>(data),
                                     len) != UWSGI_OK) {
        qCWarning(CUTELYST_UWSGI_CONN) << "Failed to write body";
        return -1;
    }
    return len;
}
