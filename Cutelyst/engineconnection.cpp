/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "engineconnection.h"

#include "common.h"

#include <Cutelyst/response_p.h>
#include <Cutelyst/Context>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CUTELYST_ENGINECONNECTION, "cutelyst.engineconnection")

using namespace Cutelyst;

EngineConnection::EngineConnection(Engine *_engine) : engine(_engine)
{

}

EngineConnection::~EngineConnection()
{
    delete body;
}

void EngineConnection::finalizeBody(Context *c)
{
    Response *response = c->response();

    if (!(status & EngineConnection::Chunked)) {
        QIODevice *body = response->bodyDevice();

        if (body) {
            body->seek(0);
            char block[64 * 1024];
            while (!body->atEnd()) {
                qint64 in = body->read(block, sizeof(block));
                if (in <= 0) {
                    break;
                }

                if (write(c, block, in) != in) {
                    qCWarning(CUTELYST_ENGINECONNECTION) << "Failed to write body";
                    break;
                }
            }
        } else {
            const QByteArray bodyByteArray = response->body();
            write(c, bodyByteArray.constData(), bodyByteArray.size());
        }
    } else if (!(status & EngineConnection::ChunkedDone)) {
        // Write the final '0' chunk
        doWrite("0\r\n\r\n", 5);
    }
}

void EngineConnection::finalizeError(Context *c)
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

void EngineConnection::finalize(Context *c)
{
    if (c->error()) {
        finalizeError(c);
    }

    if (!(status & EngineConnection::FinalizedHeaders) && !finalizeHeaders(c)) {
        return;
    }

    finalizeBody(c);
}

void EngineConnection::finalizeCookies(Context *c)
{
    Response *res = c->response();
    Headers &headers = res->headers();
    const auto cookies = res->cookies();
    for (const QNetworkCookie &cookie : cookies) {
        headers.pushHeader(QStringLiteral("set_cookie"), QString::fromLatin1(cookie.toRawForm()));
    }
}

bool EngineConnection::finalizeHeaders(Context *c)
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
    status |= EngineConnection::FinalizedHeaders;
    return writeHeaders(response->status(), headers);
}

qint64 EngineConnection::write(Context *c, const char *data, qint64 len)
{
    if (!(status & EngineConnection::Chunked)) {
        return doWrite(data, len);
    } else if (!(status & EngineConnection::ChunkedDone)) {
        const QByteArray chunkSize = QByteArray::number(len, 16).toUpper();
        QByteArray chunk;
        chunk.reserve(len + chunkSize.size() + 4);
        chunk.append(chunkSize).append("\r\n", 2)
                .append(data, len).append("\r\n", 2);

        qint64 retWrite = doWrite(chunk.data(), chunk.size());

        // Flag if we wrote an empty chunk
        if (!len) {
            status |= EngineConnection::ChunkedDone;
        }

        return retWrite == chunk.size() ? len : -1;
    }
    return -1;
}

bool EngineConnection::webSocketHandshake(Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    if (status & EngineConnection::FinalizedHeaders) {
        return false;
    }

    if (webSocketHandshakeDo(c, key, origin, protocol)) {
        status |= EngineConnection::FinalizedHeaders;
        return true;
    }

    return false;
}

bool EngineConnection::webSocketSendTextMessage(Context *c, const QString &message)
{
    Q_UNUSED(c)
    Q_UNUSED(message)
    return false;
}

bool EngineConnection::webSocketSendBinaryMessage(Context *c, const QByteArray &message)
{
    Q_UNUSED(c)
    Q_UNUSED(message)
    return false;
}

bool EngineConnection::webSocketSendPing(Context *c, const QByteArray &payload)
{
    Q_UNUSED(c)
    Q_UNUSED(payload)
    return false;
}

bool EngineConnection::webSocketClose(Context *c, quint16 code, const QString &reason)
{
    Q_UNUSED(c)
    Q_UNUSED(code)
    Q_UNUSED(reason)
    return false;
}

bool EngineConnection::webSocketHandshakeDo(Context *c, const QString &key, const QString &origin, const QString &protocol)
{
    Q_UNUSED(c)
    Q_UNUSED(key)
    Q_UNUSED(origin)
    Q_UNUSED(protocol)
    return false;
}
