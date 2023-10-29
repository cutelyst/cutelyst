/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QDateTime>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>

namespace Cutelyst {

class CUTELYST_LIBRARY Headers
{
public:
    struct HeaderKeyValue {
        QByteArray key;
        QByteArray value;

        inline bool operator==(const HeaderKeyValue &other) const
        {
            return (key.compare(other.key, Qt::CaseInsensitive) == 0) && value == other.value;
        }
    };

    /**
     * Construct an empty header object.
     */
    Headers() noexcept = default;

    /**
     * Constructs a copy of \pa other.
     */
    Headers(const Headers &other) noexcept;

#ifdef Q_COMPILER_INITIALIZER_LISTS
    /**
     * Construct a header from a std::initializer_list given by list.
     */
    inline Headers(std::initializer_list<std::pair<QByteArray, QByteArray>> list)
    {
        for (std::initializer_list<std::pair<QByteArray, QByteArray>>::const_iterator it =
                 list.begin();
             it != list.end();
             ++it)
            m_data.emplace_back(HeaderKeyValue{it->first, it->second});
    }
#endif

    /**
     * The Content-Disposition header field indicates if the content is expected
     * to be displayed inline in the browser, that is, as a Web page or as part
     * of a Web page, or as an attachment, that is downloaded and saved locally
     */
    QByteArray contentDisposition() const noexcept;

    /**
     * Defines the Cache-Control header
     */
    void setCacheControl(const QByteArray &value);

    /**
     * Defines the Content-Disposition header
     * \sa contentDisposition()
     */
    void setContentDisposition(const QByteArray &contentDisposition);

    /**
     * Defines the Content-Disposition header as type attachment and the
     * optional filename parameter
     * \sa contentDisposition()
     */
    void setContentDispositionAttachment(const QByteArray &filename = {});

    /**
     * The Content-Encoding header field is used as a modifier to the media type.
     * When present, its value indicates what additional encoding mechanism has been applied to the
     * resource.
     */
    QByteArray contentEncoding() const noexcept;

    /**
     * Defines the Content-Encoding header
     * \sa contentEncoding()
     */
    void setContentEncoding(const QByteArray &encoding);

    /**
     * The Content-Type header field indicates the media type of the message content.
     * E.g.: "text/html"
     * The value returned will be converted to lower case, and potential parameters
     * will be ignored. If there is no such header field, then the empty string is returned.
     */
    QByteArray contentType() const;

    /**
     * Defines the Content-Encoding header
     * \sa contentType()
     */
    void setContentType(const QByteArray &contentType);

    /**
     * Returns the upper-cased charset specified in the Content-Type header.
     */
    QByteArray contentTypeCharset() const;

    /**
     * The Content-Type header field indicates the media type of the message content.
     * this defines the charset of the content-type
     */
    void setContentTypeCharset(const QByteArray &charset);

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
     * Returns TRUE if the Content-Type header field indicate that the content is JSON.
     */
    bool contentIsJson() const;

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
    QByteArray setDateWithDateTime(const QDateTime &date);

    /**
     * Returns the date header as QDateTime
     */
    QDateTime date() const;

    /**
     * This header fields is used to make a request conditional. If the requested resource has
     * (or has not) been modified since the time specified in this field,
     * then the server will return a 304 Not Modified response instead of the document itself.
     */
    QByteArray ifModifiedSince() const noexcept;

    /**
     * This header fields is used to make a request conditional. If the requested resource has
     * (or has not) been modified since the time specified in this field,
     * then the server will return a 304 Not Modified response instead of the document itself.
     * This method parses the header and convert to QDateTime assuming the date is in GMT
     * timezone.
     */
    QDateTime ifModifiedSinceDateTime() const;

    /**
     * Checks if Last-Modified's content has changed in comparison to the
     * If-Modified-Since header.
     */
    bool ifModifiedSince(const QDateTime &lastModified) const;

    /**
     * Checks for If-Match header usually used on POST to avoid mid-air collisions, making sure
     * the content has not changed while the client changes it.
     * Returns true if the etag value matches the value between double quotes of the client header
     * or if the client did not provide the If-Match header.
     *
     * In case of false client should usually discard posted data and return
     * status code of 412 - Response::PreconditionFailed
     */
    bool ifMatch(const QByteArray &etag) const;

    /**
     * Checks for If-None-Match header to see if the client has the most recent
     * version of a cached resource.
     * Returns true if the etag value matches the value between double quotes of the client header.
     *
     * In case of true client should usually return an empty body along with a
     * status code of 304 - Response::NotModified.
     */
    bool ifNoneMatch(const QByteArray &etag) const;

    /**
     * Sets the ETag header in strong form by not prepending a 'W/' (weak etag)
     * This method will place the etag value between double quotes, like:
     * ETag: "33a64df551425fcc55e4d42a148795d9f25f89d4"
     */
    void setETag(const QByteArray &etag);

    /**
     * This header indicates the date and time at which the resource was last modified.
     */
    QByteArray lastModified() const noexcept;

    /**
     * Defines the date and time at which the resource was last modified.
     */
    void setLastModified(const QByteArray &value);

    /**
     * Defines the date and time at which the resource was last modified.
     * This method takes a QDateTime and write it in RFC 822 and GMT timezone.
     */
    QString setLastModified(const QDateTime &lastModified);

    /**
     * Returns the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    QByteArray server() const noexcept;

    /**
     * Defines the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    void setServer(const QByteArray &value);

    /**
     * Returns the 'Connection' header field that indicates how it should be handled after a
     * request has been processed, like 'close'.
     */
    QByteArray connection() const noexcept;

    /**
     * Returns the 'Host' header field used in request messages, containing information about which
     * host the client would like to talk to, this is especially useful for building URIs and for
     * VirtualHosts.
     */
    QByteArray host() const noexcept;

    /**
     * Returns the header field used in request messages, containing information about the user
     * agent originating the request.
     */
    QByteArray userAgent() const noexcept;

    /**
     * Used to specify the address (URI) of the document from which the requested resource address
     * was obtained.
     */
    QByteArray referer() const noexcept;

    /**
     * Sets the referrer (Referer) header.
     * This method removes the fragment from the given URI if it is present,
     * as mandated by RFC2616. Note that the removal does not happen automatically
     * if using the setHeader() method to set the referrer
     */
    void setReferer(const QByteArray &value);

    /**
     * This header must be included as part of a 401 Unauthorized response.
     * The field value consist of a challenge that indicates the authentication scheme
     * and parameters applicable to the requested URI.
     */
    void setWwwAuthenticate(const QByteArray &value);

    /**
     * This header must be included in a 407 Proxy Authentication Required response.
     */
    void setProxyAuthenticate(const QByteArray &value);

    /**
     * This method is used to get an authorization header without any decoding.
     */
    QByteArray authorization() const noexcept;

    /**
     * This method is used to get an authorization token
     */
    QByteArray authorizationBearer() const;

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return "username:password" as a single string value.
     */
    QByteArray authorizationBasic() const;

    struct Authorization {
        QString user;
        QString password;
    };

    /**
     * This method is used to get an authorization header that use the
     * "Basic Authentication Scheme".
     * It will return a pair of username and password respectively.
     */
    Authorization authorizationBasicObject() const;

    /**
     * This method is used to set an authorization header that use the
     * "Basic Authentication Scheme".
     * It won't set the values if username contains a colon ':'.
     */
    QByteArray setAuthorizationBasic(const QString &username, const QString &password);

    /**
     * A user agent that wishes to authenticate itself with a server or a proxy, may do so by
     * including these headers.
     */
    QByteArray proxyAuthorization() const noexcept;

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
    Authorization proxyAuthorizationBasicObject() const;

    /**
     * Returns the value associated with \p key
     */
    QByteArray header(QByteArrayView key) const noexcept;

    /**
     * Returns the value associated with \p key from latin1
     * \note This allocates memory so avoid when possible
     */
    QString headerAsString(QByteArrayView key) const;

    /**
     * Returns the value associated with \p key, if field is not set \p defaultValue is returned
     */
    QByteArray header(QByteArrayView key, const QByteArray &defaultValue) const noexcept;

    /**
     * Returns the value associated with \p key from latin1, if field is not set \p defaultValue is
     * returned \note This allocates memory so avoid when possible
     */
    QString headerAsString(QByteArrayView key, const QByteArray &defaultValue) const;

    /**
     * Returns all values associated with \p key
     */
    QByteArrayList headers(QByteArrayView key) const;

    /**
     * Returns all values associated with \p key
     * \note This allocates memory so avoid when possible
     */
    QStringList headersAsStrings(QByteArrayView key) const;

    /**
     * Sets the header field to value
     */
    void setHeader(const QByteArray &key, const QByteArray &value);

    /**
     * Sets the header field to the list of values
     */
    void setHeader(const QByteArray &field, const QByteArrayList &values);

    /**
     * Appends the header field to values
     */
    void pushHeader(const QByteArray &key, const QByteArray &value);

    /**
     * This method appends a header to internal data normalizing the key.
     */
    void pushHeader(const QByteArray &key, const QByteArrayList &values);

    /**
     * This method removes a header identified by \p field.
     */
    void removeHeader(QByteArrayView key);

    /**
     * Clears all headers.
     */
    inline void clear() { m_data.clear(); }

    /**
     * Returns the internal structure of headers, to be used by Engine subclasses.
     */
    inline QVector<HeaderKeyValue> data() const { return m_data; }

    /**
     * Returns true if the header field is defined.
     */
    bool contains(QByteArrayView key) const noexcept;

    QByteArrayList keys() const;

    /**
     * Returns the value associated with key.
     */
    QByteArray operator[](QByteArrayView key) const noexcept;

    /**
     * Assigns \p other to this Header and returns a reference to this Header.
     */
    inline Headers &operator=(const Headers &other) noexcept
    {
        m_data = other.m_data;
        return *this;
    }

    /**
     * Compares if another Header object has the same data as this.
     */
    bool operator==(const Headers &other) const noexcept;

private:
    QVector<HeaderKeyValue> m_data;
};

} // namespace Cutelyst

Q_DECLARE_METATYPE(Cutelyst::Headers)

QDebug CUTELYST_LIBRARY operator<<(QDebug dbg, const Cutelyst::Headers &headers);
