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

#include <Cutelyst/Headers>

#include <QVariant>
#include <QIODevice>
#include <QByteArrayMatcher>
#include <QEventLoop>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

using namespace CWSGI;

ProtocolHttp::ProtocolHttp(QObject *parent) : Protocol(parent)
{

}

void ProtocolHttp::readyRead()
{
    static QByteArrayMatcher matcher("\r\n");

    auto conn = sender();
    auto sock = qobject_cast<TcpSocket*>(conn);

    int len = sock->read(sock->buf + sock->buf_size, 4096 - sock->buf_size);

    sock->buf_size += len;

//    qDebug() << Q_FUNC_INFO << len;
//    qDebug() << Q_FUNC_INFO << QByteArray(sock->buf, sock->buf_size);

    while (sock->last < sock->buf_size) {
//        qDebug() << Q_FUNC_INFO << QByteArray(sock->buf, sock->buf_size);
        int ix = matcher.indexIn(sock->buf, sock->buf_size, sock->last);
        if (ix != -1) {
            int len = ix - sock->beginLine;
            char *ptr = sock->buf + sock->beginLine;
            sock->beginLine = ix + 2;
            sock->last = sock->beginLine;

            if (sock->connState == 0) {
                processRequest(ptr, ptr + len, sock);
                sock->connState = 1;
                sock->contentLength = -1;
                sock->headers = Cutelyst::Headers();
//                qDebug() << "--------" << sock->method << sock->path << sock->query << sock->protocol;

            } else if (sock->connState == 1) {
                if (len) {
                    processHeader(ptr, ptr + len, sock);
//                } else if (sock->contentLength != -1) {

                } else {
                    sock->processing = true;
                    sock->engine->processSocket(sock);
                    sock->processing = false;

                    if (sock->headerClose == 2) {
//                        qDebug() << "disconnectFromHost";
                        sock->disconnectFromHost();
                        break;
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
                }
            }
        } else {
            sock->last = sock->buf_size;
        }
    }

    if (sock->buf_size == 4096) {
        // 414 Request-URI Too Long
    }
}

void ProtocolHttp::processRequest(const char *ptr, const char *end, Socket *sock)
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
        ptr = word_boundary;
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

void ProtocolHttp::processHeader(const char *ptr, const char *end, Socket *sock)
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
    } else if (sock->contentLength < 0 && key.compare(QLatin1String("Connection"), Qt::CaseInsensitive) == 0) {
        bool ok;
        qint64 cl = value.toLongLong(&ok);
        if (ok && cl >= 0) {
            sock->contentLength = cl;
        }
    }
    sock->headers.pushHeader(key, value);
}

#include "moc_protocolhttp.cpp"
