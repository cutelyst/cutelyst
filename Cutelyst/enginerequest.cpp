/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "enginerequest.h"

#include "common.h"

#include <Cutelyst/response_p.h>
#include <Cutelyst/Context>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CUTELYST_ENGINEREQUEST, "cutelyst.engine_request")

using namespace Cutelyst;

EngineRequest::EngineRequest()
{

}

EngineRequest::~EngineRequest()
{
    delete body;
}

void EngineRequest::finalizeBody(Context *c)
{
    Response *response = c->response();

    if (!(status & EngineRequest::Chunked)) {
        QIODevice *body = response->bodyDevice();

        if (body) {
            body->seek(0);
            char block[64 * 1024];
            while (!body->atEnd()) {
                qint64 in = body->read(block, sizeof(block));
                if (in <= 0) {
                    break;
                }

                if (write(block, in) != in) {
                    qCWarning(CUTELYST_ENGINEREQUEST) << "Failed to write body";
                    break;
                }
            }
        } else {
            const QByteArray bodyByteArray = response->body();
            write(bodyByteArray.constData(), bodyByteArray.size());
        }
    } else if (!(status & EngineRequest::ChunkedDone)) {
        // Write the final '0' chunk
        doWrite("0\r\n\r\n", 5);
    }
}

void EngineRequest::finalizeError(Context *c)
{
    Response *res = c->response();

    res->setContentType(QStringLiteral("text/html; charset=utf-8"));

    QByteArray body;

    // Trick IE. Old versions of IE would display their own error page instead
    // of ours if we'd give it less than 512 bytes.
    body.reserve(512);

    body.append(c->errors().join(QLatin1Char('\n')).toUtf8());

    res->setBody(body);

    // Return 500
    res->setStatus(Response::InternalServerError);
}

void EngineRequest::finalize(Context *c)
{
    if (c->error()) {
        finalizeError(c);
    }

    if (!(status & EngineRequest::FinalizedHeaders) && !finalizeHeaders(c)) {
        return;
    }

    finalizeBody(c);
}

void EngineRequest::finalizeCookies(Context *c)
{
    Response *res = c->response();
    Headers &headers = res->headers();
    const auto cookies = res->cookies();
    for (const QNetworkCookie &cookie : cookies) {
        headers.pushHeader(QStringLiteral("SET_COOKIE"), QString::fromLatin1(cookie.toRawForm()));
    }
}

bool EngineRequest::finalizeHeaders(Context *c)
{
    Response *response = c->response();
    Headers &headers = response->headers();

    // Fix missing content length
    if (headers.contentLength() < 0) {
        qint64 size = response->size();
        if (size >= 0) {
            headers.setContentLength(size);
        }
    }

    finalizeCookies(c);

    // Done
    status |= EngineRequest::FinalizedHeaders;
    return writeHeaders(response->status(), headers);
}

qint64 EngineRequest::write(const char *data, qint64 len)
{
    if (!(status & EngineRequest::Chunked)) {
        return doWrite(data, len);
    } else if (!(status & EngineRequest::ChunkedDone)) {
        const QByteArray chunkSize = QByteArray::number(len, 16).toUpper();
        QByteArray chunk;
        chunk.reserve(len + chunkSize.size() + 4);
        chunk.append(chunkSize).append("\r\n", 2)
                .append(data, len).append("\r\n", 2);

        qint64 retWrite = doWrite(chunk.data(), chunk.size());

        // Flag if we wrote an empty chunk
        if (!len) {
            status |= EngineRequest::ChunkedDone;
        }

        return retWrite == chunk.size() ? len : -1;
    }
    return -1;
}

bool EngineRequest::webSocketHandshake(Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    if (status & EngineRequest::FinalizedHeaders) {
        return false;
    }

    if (webSocketHandshakeDo(c, key, origin, protocol)) {
        status |= EngineRequest::FinalizedHeaders;
        return true;
    }

    return false;
}

bool EngineRequest::webSocketSendTextMessage(const QString &message)
{
    Q_UNUSED(message)
    return false;
}

bool EngineRequest::webSocketSendBinaryMessage(const QByteArray &message)
{
    Q_UNUSED(message)
    return false;
}

bool EngineRequest::webSocketSendPing(const QByteArray &payload)
{
    Q_UNUSED(payload)
    return false;
}

bool EngineRequest::webSocketClose(quint16 code, const QString &reason)
{
    Q_UNUSED(code)
    Q_UNUSED(reason)
    return false;
}

bool EngineRequest::webSocketHandshakeDo(Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    Q_UNUSED(c)
    Q_UNUSED(key)
    Q_UNUSED(origin)
    Q_UNUSED(protocol)
    return false;
}

#include "moc_enginerequest.cpp"
