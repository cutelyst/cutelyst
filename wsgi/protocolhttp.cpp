/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "protocolhttp.h"
#include "socket.h"
#include "wsgi.h"

#include <Cutelyst/Headers>

#include <QVariant>
#include <QIODevice>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QBuffer>
#include <QTimer>
#include <QDebug>

using namespace CWSGI;

ProtocolHttp::ProtocolHttp(WSGI *wsgi, QObject *parent) : Protocol(wsgi, parent)
{
    m_postBufferSize = m_wsgi->postBufferingBufsize();
    m_bufferSize = m_wsgi->bufferSize();
    m_postBuffering = m_wsgi->postBuffering();
    m_postBuffer = new char[m_wsgi->postBufferingBufsize()];
}

ProtocolHttp::~ProtocolHttp()
{
    delete [] m_postBuffer;
}

void ProtocolHttp::readyRead()
{
    auto conn = sender();
    auto sock = qobject_cast<TcpSocket*>(conn);

    // Post buffering
    if (sock->connState == Socket::ContentBody) {
        qint64 bytesAvailable = sock->bytesAvailable();
        int len;
        qint64 remaining;

        do {
            remaining = sock->contentLength - sock->body->size();
            len = sock->read(m_postBuffer, qMin(m_postBufferSize, remaining));
            bytesAvailable -= len;
//            qDebug() << "WRITE body" << sock->contentLength << remaining << len << (remaining == len) << sock->bytesAvailable();
            sock->body->write(m_postBuffer, len);
        } while (bytesAvailable);

        if (remaining == len) {
            processRequest(sock);
        }

        return;
    }

    int len = sock->read(sock->buf + sock->buf_size, m_bufferSize - sock->buf_size);
    sock->buf_size += len;

    while (sock->last < sock->buf_size) {
//        qDebug() << Q_FUNC_INFO << QByteArray(sock->buf, sock->buf_size);
        int ix = m_matcher.indexIn(sock->buf, sock->buf_size, sock->last);
        if (ix != -1) {
            int len = ix - sock->beginLine;
            char *ptr = sock->buf + sock->beginLine;
            sock->beginLine = ix + 2;
            sock->last = sock->beginLine;

            if (sock->connState == Socket::MethodLine) {
                parseMethod(ptr, ptr + len, sock);
                sock->connState = Socket::HeaderLine;
                sock->contentLength = -1;
                sock->headers = Cutelyst::Headers();
//                qDebug() << "--------" << sock->method << sock->path << sock->query << sock->protocol;

            } else if (sock->connState == Socket::HeaderLine) {
                if (len) {
                    parseHeader(ptr, ptr + len, sock);
                } else {
                    if (sock->contentLength != -1) {
                        sock->connState = Socket::ContentBody;
                        if (m_postBuffering && sock->contentLength > m_postBuffering) {
                            auto temp = new QTemporaryFile(sock);
                            if (!temp->open()) {
                                qWarning() << "Failed to open temporary file to store post" << temp->errorString();
                                sock->disconnectFromHost();
                                return;
                            }
                            sock->body = temp;
                        } else if (m_postBuffering && sock->contentLength <= m_postBuffering) {
                            auto buffer = new QBuffer(sock);
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(sock->contentLength);
                            sock->body = buffer;
                        } else {
                            // Unbuffered
                            auto buffer = new QBuffer(sock);
                            buffer->open(QIODevice::ReadWrite);
                            buffer->buffer().reserve(sock->contentLength);
                            sock->body = buffer;
                        }

                        ptr += 2;
                        len = qMin(sock->contentLength, static_cast<qint64>(sock->buf_size - sock->last));
//                        qDebug() << "WRITE" << sock->contentLength << len;
                        if (len) {
                            sock->body->write(ptr, len);
                        }
                        sock->last += len;

                        if (sock->contentLength > len) {
//                            qDebug() << "WRITE more..." << sock->contentLength << len;
                            // need to wait for more data
                            return;
                        }
                    }

                    if (!processRequest(sock)) {
                        break;
                    }
                }
            }
        } else {
            sock->last = sock->buf_size;
        }
    }

    if (sock->buf_size == m_bufferSize) {
        // 414 Request-URI Too Long
    }
}

bool ProtocolHttp::processRequest(TcpSocket *sock)
{
//    qDebug() << "processRequest" << sock->contentLength;
    sock->processing = true;
    if (sock->body) {
        sock->body->seek(0);
    }
    sock->engine->processSocket(sock);
    sock->processing = false;

    if (sock->headerClose == 2) {
//                        qDebug() << "disconnectFromHost";
        sock->disconnectFromHost();
        return false;
    } else if (sock->last < sock->buf_size) {
        // move pipelined request to 0
        int remaining = sock->buf_size - sock->last;
        memmove(sock->buf, sock->buf + sock->last, remaining);
        sock->resetSocket();
        sock->buf_size = remaining;

        QCoreApplication::processEvents();
    } else {
        sock->resetSocket();
    }
    sock->start = sock->engine->time();

    return true;
}

void ProtocolHttp::parseMethod(const char *ptr, const char *end, Socket *sock)
{
    const char *word_boundary = ptr;
    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    sock->method = QString::fromLatin1(ptr, word_boundary - ptr);

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    // skip leading slashes
    while (*ptr == '/' && ptr <= end) {
        ++ptr;
    }

    // find path end
    while (*word_boundary != ' ' && *word_boundary != '?' && word_boundary < end) {
        ++word_boundary;
    }
    sock->path = QString::fromLatin1(ptr, word_boundary - ptr);

    if (*word_boundary == '?') {
        ptr = word_boundary + 1;
        while (*word_boundary != ' ' && word_boundary < end) {
            ++word_boundary;
        }
        sock->query = QByteArray(ptr, word_boundary - ptr);
    } else {
        sock->query = QByteArray();
    }

    // skip spaces
    while (*word_boundary == ' ' && word_boundary < end) {
        ++word_boundary;
    }
    ptr = word_boundary;

    while (*word_boundary != ' ' && word_boundary < end) {
        ++word_boundary;
    }
    sock->protocol = QString::fromLatin1(ptr, word_boundary - ptr);
}

void ProtocolHttp::parseHeader(const char *ptr, const char *end, Socket *sock)
{
    const char *word_boundary = ptr;
    while (*word_boundary != ':' && word_boundary < end) {
        ++word_boundary;
    }
    const QString key = QString::fromLatin1(ptr, word_boundary - ptr);

    while ((*word_boundary == ':' || *word_boundary == ' ') && word_boundary < end) {
        ++word_boundary;
    }
    const QString value = QString::fromLatin1(word_boundary, end - word_boundary);

    if (sock->headerClose == 0 && key.compare(QLatin1String("Connection"), Qt::CaseInsensitive) == 0) {
        if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
            sock->headerClose = 2;
        } else {
            sock->headerClose = 1;
        }
    } else if (sock->contentLength < 0 && key.compare(QLatin1String("Content-Length"), Qt::CaseInsensitive) == 0) {
        bool ok;
        qint64 cl = value.toLongLong(&ok);
        if (ok && cl >= 0) {
            sock->contentLength = cl;
        }
    }
    sock->headers.pushHeader(key, value);
}

#include "moc_protocolhttp.cpp"
