/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_export.h>
#include <Cutelyst/headers.h>

#include <QtCore/QIODevice>

class QNetworkCookie;

namespace Cutelyst {

class Context;
class Engine;
class EngineRequest;
class ResponsePrivate;
/**
 * @ingroup core
 * @class Cutelyst::Response response.h Cutelyst/Response
 * @brief A %Cutelyst response.
 *
 * A %Cutelyst response contains the data created by your application that should be
 * send back to the \link Request requesting\endlink client.
 */
class CUTELYST_EXPORT Response final : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Response)
public:
    /** This enum type specifies the status response to be sent to the client */
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
        MultiStatus                  = 207,
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
    Q_ENUM(HttpStatus)

    /** This enum type specifies the status response to be sent to the client */
    enum CloseCode {
        CloseCodeNormal                = 1000,
        CloseCodeGoingAway             = 1001,
        CloseCodeProtocolError         = 1002,
        CloseCodeDatatypeNotSupported  = 1003,
        CloseCodeReserved1004          = 1004,
        CloseCodeMissingStatusCode     = 1005,
        CloseCodeAbnormalDisconnection = 1006,
        CloseCodeWrongDatatype         = 1007,
        CloseCodePolicyViolated        = 1008,
        CloseCodeTooMuchData           = 1009,
        CloseCodeMissingExtension      = 1010,
        CloseCodeBadOperation          = 1011,
        CloseCodeTlsHandshakeFailed    = 1015
    };
    Q_ENUM(CloseCode)

    /**
     * Destroys the %Response object.
     */
    virtual ~Response() override;

    /**
     * The current response code status.
     */
    quint16 status() const noexcept;

    /**
     * Sets the response code status.
     */
    void setStatus(quint16 status) noexcept;

    /**
     * Returns true if a body device has been defined
     * as QByteArray or QIODevice or write() was called
     * and it's on chunked mode.
     */
    bool hasBody() const noexcept;

    /**
     * This function returns a reference to a
     * QByteArray which implicity sets the body
     * device to a QBuffer, even if one was already
     * set.
     */
    [[nodiscard]] QByteArray &body();

    /**
     * Returns the body IO device (if any) of this response.
     */
    QIODevice *bodyDevice() const noexcept;

    /**
     * Sets an IO device as the response \a body,
     * the open mode must be at least QIODevice::ReadOnly.
     * This function takes ownership of your device
     * deleting after the request has completed.
     */
    void setBody(QIODevice *body);

    /**
     * Sets a QByteArray as the response \a body,
     * content length will be automatically set to it's size.
     */
    void setBody(const QByteArray &body);

    /**
     * Sets a QString as the response \a body, the output will be UTF-8 and
     * content length will be automatically set to it's size.
     */
    inline void setBody(const QString &body);

    /**
     * Sets a QString as the response \a body, the output will be UTF-8 and
     * content length will be automatically set to it's size.
     */
    inline void setBody(QStringView body);

    /**
     * Sets a \a CBOR data as the response body,
     * this method is provided for convenience as it sets the content-type to application/cbor.
     */
    void setCborBody(const QByteArray &cbor);

    /**
     * Sets a \a CBOR value without transformation as the response body,
     * this method is provided for convenience as it sets the content-type to application/json.
     */
    void setCborValueBody(const QCborValue &value);

    /**
     * Sets a \a JSON string as the response body,
     * this method is provided for convenience as it sets the content-type to application/json.
     */
    inline void setJsonBody(QStringView json);

    /**
     * Sets a \a JSON string as the response body,
     * this method is provided for convenience as it sets the content-type to application/json.
     */
    void setJsonBody(const QByteArray &json);

    /**
     * Sets a QJsonObject \a obj on a QJsonDocument as the response body,
     * using toJson(QJsonDocument::Compact) output and setting
     * content-type to application/json.
     */
    void setJsonObjectBody(const QJsonObject &obj);

    /**
     * Sets a QJsonArray \a array on a QJsonDocument as the response body,
     * using toJson(QJsonDocument::Compact) output and setting
     * content-type to application/json.
     */
    void setJsonArrayBody(const QJsonArray &array);

    /**
     * Short for headers().contentEncoding();
     * \sa Headers::contentEncoding()
     * \sa setContentEncoding()
     */
    QByteArray contentEncoding() const noexcept;

    /**
     * Short for headers().setContentEncoding(encoding);
     * \sa Headers::setContentEncoding()
     * \sa contentEncoding()
     */
    void setContentEncoding(const QByteArray &encoding);

    /**
     * Short for headers().contentLength();
     * \sa Headers::contentLength()
     */
    qint64 contentLength() const;

    /**
     * Short for headers().contentType();
     * \sa Headers::contentType()
     * \sa setContentType()
     */
    QByteArray contentType() const;

    /**
     * Short for headers().setContentType(type);
     * \sa Headers::setContentType()
     * \sa contentType()
     */
    void setContentType(const QByteArray &type) { headers().setContentType(type); }

    /**
     * Short for headers().contentTypeCharset();
     * \sa Headers::contentTypeCharset()
     */
    QByteArray contentTypeCharset() const;

    /**
     * Returns the first QNetworkCookie matching the \a name
     * or a null QVariant if not found.
     */
    QVariant cookie(const QByteArray &name) const;

    /**
     * Returns a list of all cookies set.
     */
    QList<QNetworkCookie> cookies() const;

    /**
     * Defines a QNetworkCookie to be sent to the user-agent,
     * if a previous cookie->name() was set it will be replaced.
     */
    void setCookie(const QNetworkCookie &cookie);

    /**
     * Defines a list of QNetworkCookie to be sent to the user-agent,
     * all previous matches to cookie->name() will be preserved.
     */
    void setCookies(const QList<QNetworkCookie> &cookies);

    /**
     * Removes all cookies that matches \a name, returning
     * the number of cookies removed
     */
    int removeCookies(const QByteArray &name);

    /**
     * Causes the response to redirect to the specified URL. The default status is 302.
     * This is a convenience method that sets the Location header to the redirect
     * destination, and then sets the response status. You will want to return false or
     * c->detach() to interrupt the normal processing flow if you want the redirect to
     * occur straight away.
     *
     * \note Do not give a relative URL as $url, i.e: one that is not fully
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
     * \note Do not give a relative URL as $url, i.e: one that is not fully
     * qualified ("http://...", etc.) or that starts with a slash "/path/here".
     * While it may work, it is not guaranteed to do the right thing and is not a
     * standard behaviour. You may opt to use uriFor() or uriForAction() instead.
     */
    void redirect(const QString &url, quint16 status = Found);

    /**
     * Open Redirect Vulnerability is when you get an user provided URL and redirect
     * to it without checking if it's safe.
     *
     * This can be used on login forms that receive some "redir" parameter that once
     * logged in allows the user to go straight to that page instead of some home page.
     *
     * It's then possible to receive a link like
     * http://example.com/login?redir=http://exemple.com/login notice how both domain names
     * are similar for malicious porpuses, once logged in it redirects to a similar login page that
     * will pretent the auth didn't work, user might then type their credentials on that page.
     *
     * This method validades that the url scheme, domain name and port are the same of the
     * request to your server if it isn't it will send the user to \p fallback url that you
     * know it's safe. If you need to redirect the user to some other domain/port
     * validate the URL manually an use the regular \sa redirect method instead.
     */
    void redirectSafe(const QUrl &url, const QUrl &fallback);

    /**
     * Returns the HTTP location set by the redirect
     */
    QUrl location() const noexcept;

    /**
     * Shortcut headers().header()
     * \sa Headers::header()
     */
    QByteArray header(const QByteArray &field) const noexcept;

    /**
     * Shortcut headers().setHeader()
     * \sa Headers::setHeader()
     */
    void setHeader(const QByteArray &key, const QByteArray &value);

    /**
     * Returns a reference to the response headers class.
     */
    Headers &headers() noexcept;

    /**
     * Returns \c true if the request is finalized, this will be true for active websocket
     * connections because they are HTTP finalized (ie can't send an HTTP body anymore).
     */
    bool isFinalized() const noexcept;

    /**
     * Returns \c true if Headers are finalized (sent to the client).
     */
    bool isFinalizedHeaders() const noexcept;

    /**
     * Writing to user-agent is always sequential.
     */
    bool isSequential() const noexcept override;

    /**
     * Reimplemented from QIODevice::readData().
     */
    qint64 size() const noexcept override;

    /**
     * Sends the websocket handshake, if no parameters are defined it will use header data.
     * Returns true in case of success, false otherwise, which can be due missing support on
     * the engine or missing the appropriate headers.
     *
     * \note Most WebSockets client doesn't handle when a call to a WebSocket endpoint ends
     * on an HTTP authentication failure or other HTTP status that is not the update. Due that
     * it's best to always do the proper websocket handshake and then \sa webSocketClose() the
     * connection, with some meaning reason.
     */
    bool webSocketHandshake(const QByteArray &key      = {},
                            const QByteArray &origin   = {},
                            const QByteArray &protocol = {});

    /**
     * Sends a WebSocket text \a message.
     */
    bool webSocketTextMessage(const QString &message);

    /**
     * Sends a WebSocket binary \a message.
     */
    bool webSocketBinaryMessage(const QByteArray &message);

    /**
     * Sends a WebSocket ping with an optional \a payload limited to 125 bytes,
     * which will be truncated if larger.
     *
     * \note Some front-end servers will close the connection if no activity is seem, NGINX closes
     * in 60 seconds by default, in order to avoid that, sending a ping is the best to way to keep
     * the connection alive and to know that your client is still there.
     */
    bool webSocketPing(const QByteArray &payload = {});

    /**
     * Sends a WebSocket close frame, with both optional close \a code and a string \a reason.
     *
     * \note This method does not emit Request::webSocketClosed() signal. If
     * you need to track when the connection was closed, the proper way is to rely on
     * Context::destroyed() signal.
     */
    bool webSocketClose(quint16 code = Response::CloseCodeNormal, const QString &reason = {});

protected:
    /**
     * Constructs a %Response object, for engine request \a conn with \a defaultHeaders.
     */
    explicit Response(const Headers &defaultHeaders, EngineRequest *conn = nullptr);

    /**
     * Writes data to the response body, this will flush
     * all response headers first and will enter in chunked
     * mode if Transfer-Encoding header is set to chunked
     * or if no content length is set and status is
     * not 1xx or 204 (NoContent) or 304 (NotModified)
     */
    virtual qint64 writeData(const char *data, qint64 len) override;

    /**
     * Reimplemented from QIODevice::readData().
     */
    virtual qint64 readData(char *data, qint64 maxlen) override;

    ResponsePrivate *d_ptr;
    friend class Application;
    friend class Engine;
    friend class EngineConnection;
    friend class Context;
    friend class ContextPrivate;
};

inline void Response::setBody(const QString &_body)
{
    setBody(_body.toUtf8());
}

inline void Response::setBody(QStringView _body)
{
    setBody(_body.toUtf8());
}

inline void Response::setJsonBody(QStringView _body)
{
    setJsonBody(_body.toUtf8());
}

} // namespace Cutelyst
