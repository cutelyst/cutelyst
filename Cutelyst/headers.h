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

typedef struct {
    int weight;
    QByteArray key;
    QByteArray value;
} HeaderValuePair;

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

    /**
     * This header must be included as part of a 401 Unauthorized response.
     * The field value consist of a challenge that indicates the authentication scheme
     * and parameters applicable to the requested URI.
     */
    void setWwwAuthenticate(const QByteArray &value) { insert(QByteArray("WWW-Authenticate", 16), value); }

    /**
     * This header must be included in a 407 Proxy Authentication Required response.
     */
    void setProxyAuthenticate(const QByteArray &value) { insert(QByteArray("Proxy-Authenticate", 18), value); }

    /**
     * This method is used to get an authorization header without any decoding.
     */
    inline QByteArray authorization() const { return value(QByteArray("Authorization", 13)); }

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return "username:password" as a single string value.
     */
    QByteArray authorizationBasic() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return a pair of username and password respectively.
     */
    QPair<QByteArray, QByteArray> authorizationBasicPair() const;

    /**
     * This method is used to set an authorization header that use the
     * "Basic Authentication Scheme".
     * It won't set the values if username contains a colon ':'.
     */
    void setAuthorizationBasic(const QByteArray &username, const QByteArray &password);

    /**
     * A user agent that wishes to authenticate itself with a server or a proxy, may do so by including these headers.
     */
    inline QByteArray proxyAuthorization() const { return value(QByteArray("Proxy-Authorization", 19)); }

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme" but using the "Proxy-Authorization" header instead.
     * It will return "username:password" as a single string value.
     */
    QByteArray proxyAuthorizationBasic() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme" but using the "Proxy-Authorization" header instead.
     * It will return a pair of username and password respectively.
     */
    QPair<QByteArray, QByteArray> proxyAuthorizationBasicPair() const;

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
