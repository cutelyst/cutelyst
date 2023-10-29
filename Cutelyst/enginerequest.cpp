/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "enginerequest.h"

#include "common.h"

#include <Cutelyst/Context>
#include <Cutelyst/response_p.h>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CUTELYST_ENGINEREQUEST, "cutelyst.engine_request", QtWarningMsg)

using namespace Cutelyst;

EngineRequest::EngineRequest()
{
}

EngineRequest::~EngineRequest()
{
    delete context;
}

void EngineRequest::finalizeBody()
{
    if (!(status & EngineRequest::Chunked)) {
        Response *response = context->response();
        QIODevice *body    = response->bodyDevice();

        if (body) {
            if (!body->isSequential()) {
                body->seek(0);
            }

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

void EngineRequest::finalizeError()
{
    Response *res = context->response();

    res->setContentType("text/html; charset=utf-8"_qba);

    QByteArray body;

    // Trick IE. Old versions of IE would display their own error page instead
    // of ours if we'd give it less than 512 bytes.
    body.reserve(512);

    body.append(context->errors().join(QLatin1Char('\n')).toUtf8());

    res->setBody(body);

    // Return 500
    res->setStatus(Response::InternalServerError);
}

void EngineRequest::finalize()
{
    if (context->error()) {
        finalizeError();
    }

    if ((status & EngineRequest::FinalizedHeaders) || finalizeHeaders()) {
        finalizeBody();
    }

    status |= EngineRequest::Finalized;
    processingFinished();
}

void EngineRequest::finalizeCookies()
{
    Response *res      = context->response();
    Headers &headers   = res->headers();
    const auto cookies = res->cookies();
    for (const QNetworkCookie &cookie : cookies) {
        headers.pushHeader("Set-Cookie"_qba, cookie.toRawForm());
    }
}

bool EngineRequest::finalizeHeaders()
{
    Response *response = context->response();
    Headers &headers   = response->headers();

    // Fix missing content length
    if (headers.contentLength() < 0) {
        qint64 size = response->size();
        if (size >= 0) {
            headers.setContentLength(size);
        }
    }

    finalizeCookies();

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
        chunk.reserve(int(len + chunkSize.size() + 4));
        chunk.append(chunkSize).append("\r\n", 2).append(data, int(len)).append("\r\n", 2);

        qint64 retWrite = doWrite(chunk.data(), chunk.size());

        // Flag if we wrote an empty chunk
        if (!len) {
            status |= EngineRequest::ChunkedDone;
        }

        return retWrite == chunk.size() ? len : -1;
    }
    return -1;
}

bool EngineRequest::webSocketHandshake(const QByteArray &key,
                                       const QByteArray &origin,
                                       const QByteArray &protocol)
{
    if (status & EngineRequest::FinalizedHeaders) {
        return false;
    }

    if (webSocketHandshakeDo(key, origin, protocol)) {
        status |= EngineRequest::FinalizedHeaders | EngineRequest::Async | EngineRequest::IOWrite;

        context->finalize();

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

void EngineRequest::processingFinished()
{
}

bool EngineRequest::webSocketHandshakeDo(const QByteArray &key,
                                         const QByteArray &origin,
                                         const QByteArray &protocol)
{
    Q_UNUSED(key)
    Q_UNUSED(origin)
    Q_UNUSED(protocol)
    return false;
}

void EngineRequest::setPath(char *rawPath, const int len)
{
    if (len == 0) {
        path = u"/"_qs;
        return;
    }

    char *data           = rawPath;
    const char *inputPtr = data;

    bool lastSlash = false;
    bool skipUtf8  = true;
    int outlen     = 0;
    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9')
                a -= '0';
            else if (a >= 'a' && a <= 'f')
                a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F')
                a = a - 'A' + 10;

            if (b >= '0' && b <= '9')
                b -= '0';
            else if (b >= 'a' && b <= 'f')
                b = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F')
                b = b - 'A' + 10;

            *data++  = char((a << 4) | b);
            skipUtf8 = false;
        } else if (c == '+') {
            *data++ = ' ';
        } else if (c == '/') {
            // Remove duplicated slashes
            if (!lastSlash) {
                *data++ = '/';
            } else {
                --outlen;
            }
            lastSlash = true;
            continue;
        } else {
            *data++ = c;
        }
        lastSlash = false;
    }

    if (skipUtf8) {
        path = QString::fromLatin1(rawPath, outlen);
    } else {
        path = QString::fromUtf8(rawPath, outlen);
    }

    if (!path.startsWith(u'/')) {
        path.prepend(u'/');
    }
}

#include "moc_enginerequest.cpp"
