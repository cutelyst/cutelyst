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

#include <QtCore/qdatetime.h>

namespace Cutelyst {

class Headers : public QHash<QString, QString>
{
public:
    QString contentEncoding() const;

    void setContentEncoding(const QString &encoding);

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     * The value returned will be converted to lower case, and potential parameters
     * will be ignored. If there is no such header field, then the empty string is returned.
     */
    QString contentType() const;

    /**
     * Returns the upper-cased charset specified in the Content-Type header.
     */
    QString contentTypeCharset() const;

    /**
     * Returns TRUE if the Content-Type header field indicate that the content is textual.
     */
    bool contentIsText() const;

    /**
     * Returns TRUE if the Content-Type header field indicate that the
     * content is some kind of HTML (including XHTML).
     */
    bool contentIsHtml() const;

    /**
     * Returns TRUE if the Content-Type header field indicate that the content is XHTML.
     */
    bool contentIsXHtml() const;

    /**
     * Returns TRUE if the Content-Type header field indicate that the content is XML.
     */
    bool contentIsXml() const;

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     */
    void setContentType(const QString &contentType);

    /**
     * Returns the size in bytes of the message content
     */
    qint64 contentLength() const;

    /**
     * Defines the size in bytes of the message content
     */
    void setContentLength(qint64 value);

    /**
     * Defines the header that represents the date and time at which the message was originated
     */
    void setDateWithDateTime(const QDateTime &date);

    /**
     * This header fields is used to make a request conditional. If the requested resource has
     * (or has not) been modified since the time specified in this field,
     * then the server will return a 304 Not Modified response instead of the document itself.
     */
    QString ifModifiedSince() const;

    /**
     * This header fields is used to make a request conditional. If the requested resource has
     * (or has not) been modified since the time specified in this field,
     * then the server will return a 304 Not Modified response instead of the document itself.
     * This method parses the header and convert to QDateTime assuming the date is in GMT
     * timezone.
     */
    QDateTime ifModifiedSinceDateTime() const;

    /**
     * This header indicates the date and time at which the resource was last modified.
     */
    QString lastModified() const;

    /**
     * Defines the date and time at which the resource was last modified.
     */
    void setLastModified(const QString &value);

    /**
     * Defines the date and time at which the resource was last modified.
     * This method takes a QDateTime and write it in RFC 822 and GMT timezone.
     */
    void setLastModified(const QDateTime &lastModified);

    /**
     * Returns the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    QString server() const;

    /**
     * Defines the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    void setServer(const QString &value);

    QString userAgent() const;

    void setUserAgent(const QString &value);

    QString referer() const;

    /**
     * Sets the referrer (Referer) header.
     * This method removes the fragment from the given URI if it is present,
     * as mandated by RFC2616. Note that the removal does not happen automatically
     * if using the setHeader() method to set the referrer
     */
    void setReferer(const QString &value);

    /**
     * This header must be included as part of a 401 Unauthorized response.
     * The field value consist of a challenge that indicates the authentication scheme
     * and parameters applicable to the requested URI.
     */
    void setWwwAuthenticate(const QString &value);

    /**
     * This header must be included in a 407 Proxy Authentication Required response.
     */
    void setProxyAuthenticate(const QString &value);

    /**
     * This method is used to get an authorization header without any decoding.
     */
    QString authorization() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return "username:password" as a single string value.
     */
    QString authorizationBasic() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return a pair of username and password respectively.
     */
    QPair<QString, QString> authorizationBasicPair() const;

    /**
     * This method is used to set an authorization header that use the
     * "Basic Authentication Scheme".
     * It won't set the values if username contains a colon ':'.
     */
    void setAuthorizationBasic(const QString &username, const QString &password);

    /**
     * This method is used to get all the fields that
     * build a "Digest Authentication Scheme".
     * It will return a Hash containing keys and values.
     */
    QHash<QString, QString> authorizationDigest() const;

    /**
     * A user agent that wishes to authenticate itself with a server or a proxy, may do so by including these headers.
     */
    QString proxyAuthorization() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme" but using the "Proxy-Authorization" header instead.
     * It will return "username:password" as a single string value.
     */
    QString proxyAuthorizationBasic() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme" but using the "Proxy-Authorization" header instead.
     * It will return a pair of username and password respectively.
     */
    QPair<QString, QString> proxyAuthorizationBasicPair() const;

    QString header(const QString &field) const;

    QString header(const QString &field, const QString &defaultValue) const;

    /**
     * When setting a header always use setHeader instead of insert
     */
    void setHeader(const QString &field, const QString &value);

    void setHeader(const QString &field, const QStringList &values);

    void pushHeader(const QString &field, const QString &value);

    void pushHeader(const QString &field, const QStringList &values);

    void removeHeader(const QString &field);
};

}

#endif // HEADERS_H
