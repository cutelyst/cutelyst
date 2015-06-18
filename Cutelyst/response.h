/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_RESPONSE_H
#define CUTELYST_RESPONSE_H

#include <QtCore/qobject.h>

#include <Cutelyst/headers.h>

class QIODevice;
class QNetworkCookie;

namespace Cutelyst {

class Context;
class ResponsePrivate;
class Response : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Response)
    Q_ENUMS(HttpStatus)
public:
    enum HttpStatus {
        Continue                     = 100,
        SwitchingProtocols           = 101,
        OK                           = 200,
        Created                      = 201,
        Accepted                     = 202,
        NonAuthoritativeInformation  = 203,
        NoContent                    = 204,
        ResetContent                 = 205,
        PartialContent               = 206,
        MultipleChoices              = 300,
        MovedPermanently             = 301,
        Found                        = 302,
        SeeOther                     = 303, // Since HTTP/1.1
        NotModified                  = 304,
        UseProxy                     = 305, // Since HTTP/1.1
        TemporaryRedirect            = 307, // Since HTTP/1.1
        PermanentRedirect            = 308, // Since HTTP/1.1
        BadRequest                   = 400,
        Unauthorized                 = 401,
        PaymentRequired              = 402,
        Forbidden                    = 403,
        NotFound                     = 404,
        MethodNotAllowed             = 405,
        NotAcceptable                = 406,
        ProxyAuthenticationRequired  = 407,
        RequestTimeout               = 408,
        Conflict                     = 409,
        Gone                         = 410,
        LengthRequired               = 411,
        PreconditionFailed           = 412,
        RequestEntityTooLarge        = 413,
        RequestURITooLong            = 414,
        UnsupportedMediaType         = 415,
        RequestedRangeNotSatisfiable = 416,
        ExpectationFailed            = 417,
        InternalServerError          = 500,
        NotImplemented               = 501,
        BadGateway                   = 502,
        ServiceUnavailable           = 503,
        GatewayTimeout               = 504,
        HTTPVersionNotSupported      = 505,
        BandwidthLimitExceeded       = 509
    };
    explicit Response(Context *c);
    virtual ~Response();

    /**
     * The current response code status
     */
    quint16 status() const;

    /**
     * Sets the response code status
     */
    void setStatus(quint16 status);

    /**
     * Returns true if a body device has been defined.
     */
    bool hasBody() const;

    /**
     * This function returns a reference to a
     * QByteArray which implicity sets the body
     * device to a QBuffer, even if one was already
     * set.
     */
    QByteArray &body();

    /**
     * Returns the body IO device (if any) of this response.
     */
    QIODevice *bodyDevice();

    /**
     * Sets an IO device as the response body,
     * the open mode must be at least QIODevice::ReadOnly.
     * This function takes ownership of your device
     * deleting after the request has completed
     */
    void setBody(QIODevice *body);

    /**
     * Short for headers().contentEncoding();
     */
    QString contentEncoding() const;

    /**
     * Short for headers().setContentEncoding(encoding);
     */
    void setContentEncoding(const QString &encoding);

    /**
     * Short for headers().contentLength();
     */
    qint64 contentLength() const;

    /**
     * Short for headers().setContentLength(length);
     */
    void setContentLength(qint64 length)
    { headers().setContentLength(length); }

    /**
     * Short for headers().contentType();
     */
    QString contentType() const;

    /**
     * Short for headers().setContentType(type);
     */
    void setContentType(const QString &type)
    { headers().setContentType(type); }

    /**
     * Short for headers().contentTypeCharset();
     */
    QString contentTypeCharset() const;

    QList<QNetworkCookie> cookies() const;
    void addCookie(const QNetworkCookie &cookie);
    void setCookies(const QList<QNetworkCookie> &cookies);

    /**
     * Causes the response to redirect to the specified URL. The default status is 302.
     * This is a convenience method that sets the Location header to the redirect
     * destination, and then sets the response status. You will want to return false or
     * c->detach() to interrupt the normal processing flow if you want the redirect to
     * occur straight away.
     *
     * \note do not give a relative URL as $url, i.e: one that is not fully
     * qualified ("http://...", etc.) or that starts with a slash "/path/here".
     * While it may work, it is not guaranteed to do the right thing and is not a
     * standard behaviour. You may opt to use uriFor() or uriForAction() instead.
     */
    void redirect(const QUrl &url, quint16 status = Found);

    /**
     * Causes the response to redirect to the specified URL. The default status is 302.
     * This is a convenience method that sets the Location header to the redirect
     * destination, and then sets the response status. You will want to return false or
     * c->detach() to interrupt the normal processing flow if you want the redirect to
     * occur straight away.
     *
     * \note do not give a relative URL as $url, i.e: one that is not fully
     * qualified ("http://...", etc.) or that starts with a slash "/path/here".
     * While it may work, it is not guaranteed to do the right thing and is not a
     * standard behaviour. You may opt to use uriFor() or uriForAction() instead.
     */
    void redirect(const QString &url, quint16 status = Found);

    /**
     * Returns the HTTP location set by the redirect
     */
    QUrl location() const;

    /**
     * Shortcut headers().header()
     */
    QString header(const QString &field) const;

    void setHeader(const QString &field, const QString &value);

    /**
     * Returns a reference to the response headers class
     */
    Headers &headers();

    qint64 write(const char *data, qint64 len);

    qint64 write(const QByteArray &data);

protected:
    ResponsePrivate *d_ptr;
    friend class Application;
    friend class Engine;
};

}

#endif // CUTELYST_RESPONSE_H
