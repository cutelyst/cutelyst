/*
 * Copyright (C) 2014-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef HEADERS_H
#define HEADERS_H

#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QMetaType>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class CUTELYST_LIBRARY Headers
{
public:
    /**
     * Construct an empty header object.
     */
    Headers();

#ifdef Q_COMPILER_INITIALIZER_LISTS
    /**
     * Construct a header from a std::initializer_list given by list.
     */
    inline Headers(std::initializer_list<std::pair<QString,QString> > list)
    {
        for (std::initializer_list<std::pair<QString,QString> >::const_iterator it = list.begin(); it != list.end(); ++it)
            pushHeader(it->first, it->second);
    }
#endif

    /**
     * The Content-Disposition header field indicates if the content is expected
     * to be displayed inline in the browser, that is, as a Web page or as part
     * of a Web page, or as an attachment, that is downloaded and saved locally
     */
    QString contentDisposition() const;

    /**
     * Defines the Content-Disposition header
     * \sa contentDisposition()
     */
    void setContentDisposition(const QString &contentDisposition);

    /**
     * Defines the Content-Disposition header as type attachment and the
     * optional filename parameter
     * \sa contentDisposition()
     */
    void setContentDispositionAttachment(const QString &filename = QString());

    /**
     * The Content-Encoding header field is used as a modifier to the media type.
     * When present, its value indicates what additional encoding mechanism has been applied to the resource.
     */
    QString contentEncoding() const;

    /**
     * Defines the Content-Encoding header
     * \sa contentEncoding()
     */
    void setContentEncoding(const QString &encoding);

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     * The value returned will be converted to lower case, and potential parameters
     * will be ignored. If there is no such header field, then the empty string is returned.
     */
    QString contentType() const;

    /**
     * Defines the Content-Encoding header
     * \sa contentType()
     */
    void setContentType(const QString &contentType);

    /**
     * Returns the upper-cased charset specified in the Content-Type header.
     */
    QString contentTypeCharset() const;

    /**
     * The Content-Type header field indicates the media type of the message content.
     * this defines the charset of the content-type
     */
    void setContentTypeCharset(const QString &charset);

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
    QString setDateWithDateTime(const QDateTime &date);

    /**
     * Returns the date header as QDateTime
     */
    QDateTime date() const;

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
    QString setLastModified(const QDateTime &lastModified);

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

    /**
     * Returns the 'Connection' header field that indicates how it should be handled after a
     * request has been processed, like 'close'.
     */
    QString connection() const;

    /**
     * Returns the 'Host' header field used in request messages, containing information about which host the client
     * would like to talk to, this is especially useful for building URIs and for VirtualHosts.
     */
    QString host() const;

    /**
     * Returns the header field used in request messages, containing information about the user agent originating the request.
     */
    QString userAgent() const;

    /**
     * Used to specify the address (URI) of the document from which the requested resource address was obtained.
     */
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
    std::pair<QString, QString> authorizationBasicPair() const;

    /**
     * This method is used to set an authorization header that use the
     * "Basic Authentication Scheme".
     * It won't set the values if username contains a colon ':'.
     */
    QString setAuthorizationBasic(const QString &username, const QString &password);

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
    std::pair<QString, QString> proxyAuthorizationBasicPair() const;

    /**
     * Returns the value associated with \p field
     */
    QString header(const QString &field) const;

    /**
     * Returns the value associated with \p field, if field is not set \p defaultValue is returned
     */
    QString header(const QString &field, const QString &defaultValue) const;

    /**
     * Sets the header field to value
     */
    void setHeader(const QString &field, const QString &value);

    /**
     * Sets the header field to the list of values
     */
    void setHeader(const QString &field, const QStringList &values);

    /**
     * Appends the header field to values
     */
    void pushHeader(const QString &field, const QString &value);

    /**
     * This method directly inserts a Header in it's properly form
     * which is field in upper case form with underscores instead of
     * dashes, so 'Content-Type' becomes 'CONTENT_TYPE' like CGI does,
     * this method should be used only by Engines to get faster performance
     * and avoiding normalization.
     */
    inline void pushRawHeader(const QString &field, const QString &value);

    /**
     * This method appends a header to internal data normalizing the key.
     */
    void pushHeader(const QString &field, const QStringList &values);

    /**
     * This method removes a header identified by \p field.
     */
    void removeHeader(const QString &field);

    /**
     * Clears all headers.
     */
    inline void clear() {
        m_data.clear();
    }

    /**
     * Returns the internal structure of headers, to be used by Engine subclasses.
     */
    inline QHash<QString, QString> data() const {
        return m_data;
    }

    /**
     * Returns true if the header field is defined.
     */
    bool contains(const QString &field);

    /**
     * Returns the value reference associated with key.
     */
    QString &operator[](const QString &key);

    /**
     * Returns the const value associated with key.
     */
    const QString operator[](const QString &key) const;

    /**
     * Assigns \p other to this Header and returns a reference to this Header.
     */
    inline Headers &operator=(const Headers &other)
    {
        m_data = other.m_data;
        return *this;
    }

    /**
     * Compares if another Header object has the same data as this.
     */
    inline bool operator==(const Headers &other) const {
        return m_data == other.m_data;
    }

    /**
     * Compares if another Header object does not have the same data as this.
     */
    inline bool operator!=(const Headers &other) const {
        return m_data != other.m_data;
    }

    /**
     * Returns this Header internal data as a QVariant for easiness with Q_PROPERTY.
     */
    inline operator QVariant() const {
        return QVariant::fromValue(m_data);
    }

private:
    QHash<QString, QString> m_data;
};

void Headers::pushRawHeader(const QString &field, const QString &value) {
    m_data.insertMulti(field, value);
}

}

Q_DECLARE_METATYPE(Cutelyst::Headers)

#endif // HEADERS_H
