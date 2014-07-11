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
    QByteArray contentEncoding() const { return value("Content-Encoding"); }

    void setContentEncoding(const QByteArray &encoding);

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     */
    inline QByteArray contentType() const { return value("Content-Type"); }

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     */
    void setContentType(const QByteArray &contentType);

    /**
     * Returns the size in bytes of the message content
     */
    inline qint64 contentLength() const { return value("Content-Length").toLongLong(); }

    /**
     * Defines the size in bytes of the message content
     */
    void setContentLength(qint64 value);

    /**
     * This header represents the date and time at which the message was originated
     */
    QDateTime date() const;

    /**
     * Defines the header that represents the date and time at which the message was originated
     */
    void setDate(const QDateTime &date);

    /**
     * Returns the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    inline QByteArray server() const { return value("Server"); }

    /**
     * Defines the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    void setServer(const QByteArray &value);

    inline QByteArray userAgent() const { return value("User-Agent"); }

    void setUserAgent(const QByteArray &value);

    inline QByteArray referer() const { return value("Referer"); }

    void setReferer(const QByteArray &value) { insert("Referer", value); }

    QByteArray header(const QByteArray &field) const { return value(field); }

    void setHeader(const QString &field, const QStringList &values);
    void setHeader(const QByteArray &field, const QByteArray &value);

    /**
     * Returns the hearder in the order suggested by HTTP RFC's
     * "good pratices", this function is mainly used by the Engine class
     */
    QList<HeaderValuePair> headersForResponse() const;
};

}

#endif // HEADERS_H
