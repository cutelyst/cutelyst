/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef HEADERS_H
#define HEADERS_H

#include <QDate>

namespace Cutelyst {

typedef QPair<QByteArray, QByteArray> HeaderValuePair;

class Headers : public QHash<QByteArray, QByteArray>
{
public:
    inline QByteArray contentEncoding() const { return value(QByteArray("Content-Encoding", 16)); }

    inline void setContentEncoding(const QByteArray &encoding) { insert(QByteArray("Content-Encoding", 16), encoding); }

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     */
    inline QByteArray contentType() const { return value(QByteArray("Content-Type", 12)); }

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     */
    inline void setContentType(const QByteArray &contentType) { insert(QByteArray("Content-Type", 12), contentType); }

    /**
     * Returns the size in bytes of the message content
     */
    inline qint64 contentLength() const { return value(QByteArray("Content-Length", 14)).toLongLong(); }

    /**
     * Defines the size in bytes of the message content
     */
    inline void setContentLength(qint64 value) { insert(QByteArray("Content-Length", 14), QByteArray::number(value)); }

    /**
     * Defines the header that represents the date and time at which the message was originated
     */
    void setDateWithDateTime(const QDateTime &date);

    /**
     * This header fields is used to make a request conditional. If the requested resource has
     * (or has not) been modified since the time specified in this field,
     * then the server will return a 304 Not Modified response instead of the document itself.
     */
    inline QByteArray ifModifiedSince() const { return value(QByteArray("If-Modified-Since", 17)); }

    /**
     * This header indicates the date and time at which the resource was last modified.
     */
    inline QByteArray lastModified() const { return value(QByteArray("Last-Modified", 13)); }

    /**
     * Defines the date and time at which the resource was last modified.
     * This method takes a QDateTime and write it in RFC 822 and GMT timezone.
     */
    inline void setLastModified(const QByteArray &value) { insert(QByteArray("Last-Modified", 13), value); }

    /**
     * Defines the date and time at which the resource was last modified.
     * This method takes a QDateTime and write it in RFC 822 and GMT timezone.
     */
    void setLastModifiedDateTime(const QDateTime &lastModified);

    /**
     * Returns the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    inline QByteArray server() const { return value(QByteArray("Server", 6)); }

    /**
     * Defines the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    inline void setServer(const QByteArray &value) { insert(QByteArray("Server", 6), value); }

    inline QByteArray userAgent() const { return value(QByteArray("User-Agent", 10)); }

    inline void setUserAgent(const QByteArray &value) { insert(QByteArray("User-Agent", 10), value); }

    inline QByteArray referer() const { return value(QByteArray("Referer", 7)); }

    void setReferer(const QByteArray &value) { insert(QByteArray("Referer", 7), value); }

    QByteArray header(const QByteArray &field) const { return value(field); }

    void setHeader(const QString &field, const QStringList &values);

    void setHeader(const QByteArray &field, const QByteArray &value) { insert(field, value); }

    /**
     * Returns the hearder in the order suggested by HTTP RFC's
     * "good pratices", this function is mainly used by the Engine class
     */
    QList<HeaderValuePair> headersForResponse() const;
};

}

#endif // HEADERS_H
