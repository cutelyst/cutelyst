/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_REQUEST_H
#define CUTELYST_REQUEST_H

#include <Cutelyst/cutelyst_export.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/paramsmultimap.h>

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

class QIODevice;
class QHostAddress;
class QNetworkCookie;

namespace Cutelyst {

class Engine;
class Upload;
class Context;

/**
 * \related Upload
 * A vector of Upload pointers.
 */
typedef QVector<Upload *> Uploads;

class EngineRequest;
class RequestPrivate;
/**
 * @ingroup core
 * @class Cutelyst::Request request.h Cutelyst/Request
 * @brief A request.
 *
 * A request contains the data that should be handled by your application
 * for the client.
 */
class CUTELYST_EXPORT Request final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(quint16 port READ port CONSTANT)
    Q_PROPERTY(QUrl uri READ uri CONSTANT)
    Q_PROPERTY(QString base READ base CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QString match READ match CONSTANT)
    Q_PROPERTY(QStringList arguments READ arguments CONSTANT)
    Q_PROPERTY(QStringList args READ arguments CONSTANT)
    Q_PROPERTY(bool secure READ secure CONSTANT)
    Q_PROPERTY(QVariant bodyData READ bodyData CONSTANT)
    Q_PROPERTY(Cutelyst::ParamsMultiMap bodyParams READ bodyParameters CONSTANT)
    Q_PROPERTY(Cutelyst::ParamsMultiMap queryParams READ queryParameters CONSTANT)
    Q_PROPERTY(QByteArray contentEncoding READ contentEncoding CONSTANT)
    Q_PROPERTY(QByteArray contentType READ contentType CONSTANT)
    Q_PROPERTY(QByteArray method READ method CONSTANT)
    Q_PROPERTY(QByteArray protocol READ protocol CONSTANT)
    Q_PROPERTY(QByteArray userAgent READ userAgent CONSTANT)
    Q_PROPERTY(QByteArray referer READ referer CONSTANT)
    Q_PROPERTY(QString remoteUser READ remoteUser CONSTANT)
public:
    /**
     * Destroys the %Request object.
     */
    virtual ~Request();

    /**
     * Returns the address of the client.
     */
    [[nodiscard]] QHostAddress address() const noexcept;

    /**
     * Returns the address as string of the client.
     *
     * When using IPv6 sockets to listen to both IPv4 and IPv6 sockets the
     * output of QHostAddress might be something like "::ffff:127.0.0.1", which
     * is not a bug and a valid address, but from an application point of view
     * this might not be interesting to have, so this function checks if it's
     * possible to convert to an IPv4 address and returns "127.0.0.1".
     */
    [[nodiscard]] QString addressString() const;

    /**
     * Returns the hostname of the client,
     * or null if not found or an error has happened.
     *
     * This functions makes blocking call to do a reverse
     * DNS lookup if the engine didn't set the hostname.
     */
    [[nodiscard]] QString hostname() const;

    /**
     * Returns the originating port of the client
     */
    [[nodiscard]] quint16 port() const noexcept;

    /**
     * Returns the uri as close as possible to what
     * the user has in his browser url.
     */
    [[nodiscard]] QUrl uri() const;

    /**
     * Returns Contains the URI base. This will always have a trailing slash.
     * Note that the URI scheme (e.g., http vs. https) must be determined through
     * heuristics; depending on your server configuration, it may be incorrect.
     * See secure() for more info.
     *
     * If your application was queried with the URI http://localhost:3000/some/path
     * then base is 'http://localhost:3000'.
     */
    [[nodiscard]] QString base() const;

    /**
     * Returns the path, i.e. the part of the URI after base(), for the current request.
     * for http://localhost/path/foo path will contain '/path/foo'.
     */
    [[nodiscard]] QString path() const noexcept;

    /**
     * This contains the matching part of a Regex action.
     * Otherwise it returns the same as 'action' (not a pointer but it's private name),
     * except for default actions, which return an empty string.
     */
    [[nodiscard]] QString match() const noexcept;

    /**
     * Defines the matching part of the request
     * Useful for custom dispatchers
     */
    void setMatch(const QString &match);

    /**
     * Returns a list of string containing the arguments.
     * For example, if your action was
     * \code{.h}
     * class foo : public Cutelyst::Controller
     * {
     * public:
     *   C_ATTR(bar, :Local)
     *   void bar(Context *c);
     * };
     * \endcode
     * and the URI for the request was http://.../foo/bar/bah, the string
     * 'bah' would be the first and only argument.
     *
     * Arguments get automatically URI-unescaped for you.
     */
    [[nodiscard]] QStringList arguments() const noexcept;

    /**
     * Defines the arguments of the request.
     * Useful for custom dispatchers and/or actions.
     */
    void setArguments(const QStringList &arguments);

    /**
     * Shortcut for arguments().
     */
    [[nodiscard]] inline QStringList args() const noexcept;

    /**
     * Returns captures.
     */
    [[nodiscard]] QStringList captures() const noexcept;

    /**
     * Defines the captures of the request
     * Useful for custom dispatchers and/or actions
     */
    void setCaptures(const QStringList &captures);

    /**
     * Returns \c true or \c false to indicate that a connection is secure (https).
     * The reliability of it may depend on your server configuration. %Cutelyst
     * relies on the Engine to set this information which is used to build up
     * uri().scheme(). The Engine itself might not be aware of a front HTTP
     * server with https enabled.
     */
    [[nodiscard]] bool secure() const noexcept;

    /**
     * Returns the message body of the request as
     * passed by the Engine, this can even be a file
     * if the Engine wants to.
     */
    [[nodiscard]] QIODevice *body() const noexcept;

    /**
     * Returns a QVariant representation of POST/PUT body data that is not classic HTML
     * form data, such as JSON, XML, etc. By default, %Cutelyst will parse incoming
     * data of the type 'application/json' and return access to that data via this method.
     *
     * You may define addition data_handlers.
     *
     * If the POST is malformed in some way (such as undefined or not content that matches
     * the content-type) we return a null QVariant.
     *
     * If the POSTed content type does not match an available data handler,
     * this will also return a null QVariant.
     */
    [[nodiscard]] QVariant bodyData() const;

    /**
     * When request Content-Type is 'application/cbor' this will
     * contain the parsed CBOR value.
     */
    [[nodiscard]] QCborValue bodyCbor() const;

    /**
     * When request Content-Type is 'application/json' this will
     * contain the parsed JSON representation document.
     */
    [[nodiscard]] QJsonDocument bodyJsonDocument() const;

    /**
     * When request Content-Type is 'application/json' this will
     * contain the parsed JSON representation object.
     */
    [[nodiscard]] QJsonObject bodyJsonObject() const;

    /**
     * When request Content-Type is 'application/json' this will
     * contain the parsed JSON representation array.
     */
    [[nodiscard]] QJsonArray bodyJsonArray() const;

    /**
     * Returns a QVariantMap of body (POST/PUT) parameters, this method
     * is expensive as it creates the map each time it's called, cache
     * it's result instead of calling multiple times
     */
    [[nodiscard]] QVariantMap bodyParametersVariant() const;

    /**
     * Returns a Map of body (POST/PUT) parameters when content type is
     * application/x-www-form-urlencoded
     */
    [[nodiscard]] ParamsMultiMap bodyParameters() const;

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    [[nodiscard]] inline QString bodyParameter(const QString &key,
                                               const QString &defaultValue = {}) const;

    /**
     * Convenience method for geting all body values passing a key
     *
     * \note Unlike QMap::values() this return values in insertion order.
     */
    [[nodiscard]] QStringList bodyParameters(const QString &key) const;

    /**
     * Short for bodyParameters()
     */
    [[nodiscard]] inline ParamsMultiMap bodyParams() const;

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    [[nodiscard]] inline QString bodyParam(const QString &key,
                                           const QString &defaultValue = {}) const;

    /**
     * Convenience method for geting all body values passing a key
     *
     * \note Unlike QMap::values() this return values in insertion order.
     */
    [[nodiscard]] inline QStringList bodyParams(const QString &key) const;

    /**
     * Contains the keywords portion of a query string, when no '=' signs are present.
     * \code
     * http://localhost/path?some+keywords
     * c->request()->queryKeywords() will contain 'some keywords'
     * \endcode
     */
    [[nodiscard]] QString queryKeywords() const;

    /**
     * Returns a QVariantMap of query string parameters, this method
     * is expensive as it creates the map each time it's called, cache
     * it's result instead of calling multiple times
     */
    [[nodiscard]] QVariantMap queryParametersVariant() const;

    /**
     * Returns a QMultiHash containing the query string parameters
     */
    [[nodiscard]] ParamsMultiMap queryParameters() const;

    /**
     * Convenience method for geting a single query value passing a key and an optional default
     * value
     */
    [[nodiscard]] inline QString queryParameter(const QString &key,
                                                const QString &defaultValue = {}) const;

    /**
     * Convenience method for geting all query values passing a key
     *
     * \note Unlike QMap::values() this return values in insertion order.
     */
    [[nodiscard]] QStringList queryParameters(const QString &key) const;

    /**
     * Short for queryParameters()
     */
    [[nodiscard]] inline ParamsMultiMap queryParams() const;

    /**
     * Convenience method for geting a single query value passing a key and an optional default
     * value
     */
    [[nodiscard]] inline QString queryParam(const QString &key,
                                            const QString &defaultValue = {}) const;

    /**
     * Convenience method for geting all query values passing a key
     *
     * \note Unlike QMap::values() this return values in insertion order.
     */
    [[nodiscard]] inline QStringList queryParams(const QString &key) const;

    /**
     * Returns the Content-Encoding header
     */
    [[nodiscard]] inline QByteArray contentEncoding() const noexcept;

    /**
     * Returns the Content-Type header
     */
    [[nodiscard]] inline QByteArray contentType() const;

    struct Cookie {
        QByteArray name;
        QByteArray value;
    };

    /**
     * Returns the first cookie value with the given name
     */
    [[nodiscard]] QByteArray cookie(QByteArrayView name) const;

    /**
     * Returns a list of cookies that match with the given name
     *
     * \note this return values in insertion order.
     */
    [[nodiscard]] QByteArrayList cookies(QByteArrayView name) const;

    /**
     * Returns all the cookies from the request
     */
    [[nodiscard]] QMultiMap<QByteArrayView, Cookie> cookies() const;

    /**
     * Short for headers().header(key);
     */
    [[nodiscard]] inline QByteArray header(QByteArrayView key) const noexcept;

    /**
     * Returns the HTTP request headers
     */
    [[nodiscard]] Headers headers() const noexcept;

    /**
     * Returns the request method (GET, POST, HEAD, etc).
     */
    [[nodiscard]] QByteArray method() const noexcept;

    /**
     * Returns true if the request method is POST.
     */
    [[nodiscard]] bool isPost() const noexcept;

    /**
     * Returns true if the request method is GET.
     */
    [[nodiscard]] bool isGet() const noexcept;

    /**
     * Returns true if the request method is HEAD.
     */
    [[nodiscard]] bool isHead() const noexcept;

    /**
     * Returns true if the request method is PUT.
     */
    [[nodiscard]] bool isPut() const noexcept;

    /**
     * Returns true if the request method is PATCH.
     */
    [[nodiscard]] bool isPatch() const noexcept;

    /**
     * Returns true if the request method is DELETE.
     */
    [[nodiscard]] bool isDelete() const noexcept;

    /**
     * Returns the protocol (HTTP/1.0 or HTTP/1.1) used for the current request.
     */
    [[nodiscard]] QByteArray protocol() const noexcept;

    /**
     * Returns true if the request's X-Requested-With header field is "XMLHttpRequest",
     * indicating that the request was issued by a client library such as jQuery.
     */
    [[nodiscard]] bool xhr() const noexcept;

    /**
     * Returns the user agent (browser) version string.
     */
    [[nodiscard]] inline QByteArray userAgent() const noexcept;

    /**
     * referer Shortcut for header("Referer")
     */
    [[nodiscard]] inline QByteArray referer() const noexcept;

    /**
     * Returns the value of the REMOTE_USER environment variable.
     */
    [[nodiscard]] QString remoteUser() const noexcept;

    /**
     * Returns a vector containing uploads as provided by a multipart/form-data content type
     */
    [[nodiscard]] QVector<Upload *> uploads() const;

    /**
     * Returns a map containing uploads, where their key is
     * the field name.
     */
    [[nodiscard]] QMultiMap<QStringView, Upload *> uploadsMap() const;

    /**
     * Returns all (if any) Upload objects for the given field.
     */
    [[nodiscard]] Uploads uploads(QStringView name) const;

    /**
     * Returns the first Upload object for the given field,
     * if no upload matches the field name this function
     * returns 0.
     */
    [[nodiscard]] inline Upload *upload(QStringView name) const;

    /**
     * Returns a ParamsMultiMap of parameters stemming from the current request's params,
     * plus the ones supplied.  Keys for which no current param exists will be
     * added, keys with null values will be removed and keys with existing
     * params will be replaced.
     * Note that you can supply a true value as the final
     * argument to change behavior with regards to existing parameters, appending
     * values rather than replacing them.
     *
     * A quick example:
     * \code{.cpp}
     * // URI query params foo=1
     * ParamsMultiMap params = request->mangleParams({ {"foo", "2"} });
     * // Result is query params of foo=2
     * \endcode
     * versus append mode:
     * \code{.cpp}
     * // URI query params foo=1
     * ParamsMultiMap params = request->mangleParams({ {"foo", "2"} }, true);
     * // Result is query params of foo=1&foo=2
     * \endcode
     * This is the code behind uriWith().
     */
    [[nodiscard]] ParamsMultiMap mangleParams(const ParamsMultiMap &args,
                                              bool append = false) const;

    /**
     * Returns a rewritten URI object for the current request. Key/value pairs
     * passed in will override existing parameters. You can remove an existing
     * parameter by passing in an undef value. Unmodified pairs will be
     * preserved.
     *
     * You may also pass an optional second parameter that puts uriWith() into
     * append mode:
     * \code{.cpp}
     * req->uriWith({ {"key", "value"} }, true);
     * \endcode
     * See mangleParams() for an explanation of this behavior.
     */
    [[nodiscard]] QUrl uriWith(const ParamsMultiMap &args, bool append = false) const;

    /**
     * Returns the current Engine processing the requests.
     */
    [[nodiscard]] Engine *engine() const noexcept;

    /**
     * Constructs a new %Request object.
     */
    Request(EngineRequest *engineRequest);

Q_SIGNALS:
    /*!
     * Emitted when the websocket receives a text frame, this is usefull for parsing
     * big chunks of data without waiting till the whole message arives.
     */
    void webSocketTextFrame(const QString &message, bool isLastFrame, Cutelyst::Context *c);

    /*!
     * Emitted when the websocket receives a text message, this accounts for all text frames till
     * the last one.
     */
    void webSocketTextMessage(const QString &message, Cutelyst::Context *c);

    /*!
     * Emitted when the websocket receives a binary frame, this is usefull for parsing
     * big chunks of data without waiting till the whole message arives.
     */
    void webSocketBinaryFrame(const QByteArray &message, bool isLastFrame, Cutelyst::Context *c);

    /*!
     * Emitted when the websocket receives a binary message, this accounts for all binary frames
     * till the last one.
     */
    void webSocketBinaryMessage(const QByteArray &message, Cutelyst::Context *c);

    /*!
     * Emitted when the websocket receives a pong frame, which might include a payload
     */
    void webSocketPong(const QByteArray &payload, Cutelyst::Context *c);

    /*!
     * Emitted when the websocket receives a close frame, including a close code and a reason,
     * it's also emitted when the connection closes without the client sending the close frame.
     *
     * \note This signal is NOT emitted when explicit calling \sa Response::webSocketClose(). If
     * you need to track when the connection was closed, the proper way is to rely on
     * Context::destroyed() signal.
     */
    void webSocketClosed(quint16 closeCode, const QString &reason);

protected:
    RequestPrivate *d_ptr;

private:
    friend class Application;
    friend class Dispatcher;
    friend class DispatchType;
    friend class Context;
    Q_DECLARE_PRIVATE(Request)
};

inline QStringList Request::args() const noexcept
{
    return arguments();
}

inline QString Request::bodyParameter(const QString &key, const QString &defaultValue) const
{
    return bodyParameters().value(key, defaultValue);
}

inline ParamsMultiMap Request::bodyParams() const
{
    return bodyParameters();
}

inline QString Request::bodyParam(const QString &key, const QString &defaultValue) const
{
    return bodyParameters().value(key, defaultValue);
}

inline QStringList Request::bodyParams(const QString &key) const
{
    return bodyParameters(key);
}

inline QString Request::queryParameter(const QString &key, const QString &defaultValue) const
{
    return queryParameters().value(key, defaultValue);
}

inline ParamsMultiMap Request::queryParams() const
{
    return queryParameters();
}

inline QString Request::queryParam(const QString &key, const QString &defaultValue) const
{
    return queryParameters().value(key, defaultValue);
}

inline QStringList Request::queryParams(const QString &key) const
{
    return queryParameters(key);
}

inline QByteArray Request::contentEncoding() const noexcept
{
    return headers().contentEncoding();
}

inline QByteArray Request::contentType() const
{
    return headers().contentType();
}

inline QByteArray Request::header(QByteArrayView key) const noexcept
{
    return headers().header(key);
}

inline QByteArray Request::userAgent() const noexcept
{
    return headers().userAgent();
}

inline QByteArray Request::referer() const noexcept
{
    return headers().referer();
}

inline Upload *Request::upload(QStringView name) const
{
    return uploadsMap().value(name);
}

} // namespace Cutelyst

#endif // CUTELYST_REQUEST_H
