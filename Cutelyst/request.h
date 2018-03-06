/*
 * Copyright (C) 2013-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_REQUEST_H
#define CUTELYST_REQUEST_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>
#include <Cutelyst/headers.h>

class QIODevice;
class QHostAddress;

namespace Cutelyst {

class Engine;
class Upload;
class Context;

/** A vector of Upload pointers */
typedef QVector<Upload *> Uploads;

class RequestPrivate;
class CUTELYST_LIBRARY Request : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname)
    Q_PROPERTY(quint16 port READ port)
    Q_PROPERTY(QUrl uri READ uri)
    Q_PROPERTY(QString base READ base)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString match READ match)
    Q_PROPERTY(QStringList arguments READ arguments)
    Q_PROPERTY(QStringList args READ arguments)
    Q_PROPERTY(bool secure READ secure)
    Q_PROPERTY(QVariant bodyData READ bodyData)
    Q_PROPERTY(Cutelyst::ParamsMultiMap bodyParams READ bodyParameters)
    Q_PROPERTY(Cutelyst::ParamsMultiMap queryParams READ queryParameters)
    Q_PROPERTY(QVariant headers READ headers())
    Q_PROPERTY(QString contentEncoding READ contentEncoding)
    Q_PROPERTY(QString contentType READ contentType)
    Q_PROPERTY(QString method READ method)
    Q_PROPERTY(QString protocol READ protocol)
    Q_PROPERTY(QString userAgent READ userAgent)
    Q_PROPERTY(QString referer READ referer)
    Q_PROPERTY(QString remoteUser READ remoteUser)
public:
    virtual ~Request();

    /**
     * Returns the address of the client
     */
    QHostAddress address() const;

    /**
     * Returns the address as string of the client.
     *
     * When using IPv6 sockets to listen to both IPv4 and IPv6 sockets the
     * output of QHostAddress might be something like "::ffff:127.0.0.1", which
     * is not a bug and a valid address, but from an application point of view
     * this might not be interesting to have, so this function checks if it's
     * possible to convert to an IPv4 address and returns "127.0.0.1".
     */
    QString addressString() const;

    /**
     * Returns the hostname of the client,
     * or null if not found or an error has happened.
     *
     * This functions makes blocking call to do a reverse
     * DNS lookup if the engine didn't set the hostname.
     */
    QString hostname() const;

    /**
     * Returns the originating port of the client
     */
    quint16 port() const;

    /**
     * Returns the uri as close as possible to what
     * the user has in his browser url.
     */
    QUrl uri() const;

    /**
     * Returns Contains the URI base. This will always have a trailing slash.
     * Note that the URI scheme (e.g., http vs. https) must be determined through
     * heuristics; depending on your server configuration, it may be incorrect.
     * See secure() for more info.
     *
     * If your application was queried with the URI http://localhost:3000/some/path
     * then base is http://localhost:3000/.
     */
    QString base() const;

    /**
     * Returns the path, i.e. the part of the URI after base(), for the current request.
     * for  http://localhost/path/foo
     * path will contain 'path/foo'
     */
    QString path() const;

    /**
     * This contains the matching part of a Regex action.
     * Otherwise it returns the same as 'action' (not a pointer but it's private name),
     * except for default actions, which return an empty string.
     */
    QString match() const;

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
     * bah would be the first and only argument.
     *
     * Arguments get automatically URI-unescaped for you.
     */
    QStringList arguments() const;

    /**
     * Defines the arguments of the request
     * Useful for custom dispatchers and/or actions
     */
    void setArguments(const QStringList &arguments);

    /**
     * Shortcut for arguments()
     */
    inline QStringList args() const;

    /**
     * Captures
     */
    QStringList captures() const;

    /**
     * Defines the arguments of the request
     * Useful for custom dispatchers and/or actions
     */
    void setCaptures(const QStringList &captures);

    /**
     * Returns true or false to indicate that a connection is secure (https),
     * The reliability of it may depend on your server configuration, Cutelyst
     * relies on the Engine to set this information which is used to build up
     * uri().scheme(). The Engine itself might not be aware of a front HTTP
     * server with https enabled.
     */
    bool secure() const;

    /**
     * Returns the message body of the request as
     * passed by the Engine, this can even be a file
     * if the Engine wants to.
     */
    QIODevice *body() const;

    /**
     * Returns a QVariant representation of POST/PUT body data that is not classic HTML
     * form data, such as JSON, XML, etc. By default, Cutelyst will parse incoming
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
    QVariant bodyData() const;

    /**
     * Returns a QVariantMap of body (POST/PUT) parameters, this method
     * is expensive as it creates the map each time it's called, cache
     * it's result instead of calling multiple times
     */
    QVariantMap bodyParametersVariant() const;

    /**
     * Returns a Map of body (POST/PUT) parameters when content type is application/x-www-form-urlencoded
     */
    ParamsMultiMap bodyParameters() const;

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    inline QString bodyParameter(const QString &key, const QString &defaultValue = QString()) const;

    /**
     * Convenience method for geting all body values passing a key
     */
    inline QStringList bodyParameters(const QString &key) const;

    /**
     * Short for bodyParameters()
     */
    inline ParamsMultiMap bodyParams() const;

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    inline QString bodyParam(const QString &key, const QString &defaultValue = QString()) const;

    /**
     * Convenience method for geting all body values passing a key
     */
    inline QStringList bodyParams(const QString &key) const;

    /**
     * Contains the keywords portion of a query string, when no '=' signs are present.
     * * \code
     * http://localhost/path?some+keywords
     * c->request()->queryKeywords() will contain 'some keywords'
     * \endcode
     */
    QString queryKeywords() const;

    /**
     * Returns a QVariantMap of query string parameters, this method
     * is expensive as it creates the map each time it's called, cache
     * it's result instead of calling multiple times
     */
    QVariantMap queryParametersVariant() const;

    /**
     * Returns a QMultiHash containing the query string parameters
     */
    ParamsMultiMap queryParameters() const;

    /**
     * Convenience method for geting a single query value passing a key and an optional default value
     */
    inline QString queryParameter(const QString &key, const QString &defaultValue = QString()) const;

    /**
     * Convenience method for geting all query values passing a key
     */
    inline QStringList queryParameters(const QString &key) const;

    /**
     * Short for queryParameters()
     */
    inline ParamsMultiMap queryParams() const;

    /**
     * Convenience method for geting a single query value passing a key and an optional default value
     */
    inline QString queryParam(const QString &key, const QString &defaultValue = QString()) const;

    /**
     * Convenience method for geting all query values passing a key
     */
    inline QStringList queryParams(const QString &key) const;

    /**
     * Returns the Content-Encoding header
     */
    inline QString contentEncoding() const;

    /**
     * Returns the Content-Type header
     */
    inline QString contentType() const;

    /**
     * Returns the cookie with the given name
     */
    QString cookie(const QString &name) const;

    /**
     * Returns all the cookies from the request
     */
    QMap<QString, QString> cookies() const;

    /**
     * Short for headers().header(key);
     */
    inline QString header(const QString &key) const;

    /**
     * Returns the HTTP request headers
     */
    Headers headers() const;

    /**
     * Returns the request method (GET, POST, HEAD, etc).
     */
    QString method() const;

    /**
     * Returns true if the request method is POST.
     */
    bool isPost() const;

    /**
     * Returns true if the request method is GET.
     */
    bool isGet() const;

    /**
     * Returns the protocol (HTTP/1.0 or HTTP/1.1) used for the current request.
     */
    QString protocol() const;

    /**
     * Returns the user agent (browser) version string.
     */
    inline QString userAgent() const;

    /**
     * referer Shortcut for header("Referer")
     */
    inline QString referer() const;

    /**
     * Returns the value of the REMOTE_USER environment variable.
     */
    QString remoteUser() const;

    /**
     * Returns a vector containing uploads as provided by a multipart/form-data content type
     */
    QVector<Upload *> uploads() const;

    /**
     * Returns a map containing uploads, where their key is
     * the field name.
     */
    QMap<QString, Upload *> uploadsMap() const;

    /**
     * Returns all (if any) Upload objects for the given field.
     */
    Uploads uploads(const QString &name) const;

    /**
     * Returns the first Upload object for the given field,
     * if no upload matches the field name this function
     * returns 0.
     */
    inline Upload *upload(const QString &name) const;

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
    ParamsMultiMap mangleParams(const ParamsMultiMap &args, bool append = false) const;

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
    QUrl uriWith(const ParamsMultiMap &args, bool append = false) const;

    /**
     * Returns the current Engine processing the requests.
     */
    Engine *engine() const;

    /**
     * Constructs a new Request object.
     */
    Request(RequestPrivate *prv);

Q_SIGNALS:
    /*!
     * Emitted when the websocket receives a text frame, this is usefull for parsing
     * big chunks of data without waiting till the whole message arives.
     */
    void webSocketTextFrame(const QString &message, bool isLastFrame, Context *c);

    /*!
     * Emitted when the websocket receives a text message, this accounts for all text frames till the last one.
     */
    void webSocketTextMessage(const QString &message, Context *c);

    /*!
     * Emitted when the websocket receives a binary frame, this is usefull for parsing
     * big chunks of data without waiting till the whole message arives.
     */
    void webSocketBinaryFrame(const QByteArray &message, bool isLastFrame, Context *c);

    /*!
     * Emitted when the websocket receives a binary message, this accounts for all binary frames till the last one.
     */
    void webSocketBinaryMessage(const QByteArray &message, Context *c);

    /*!
     * Emitted when the websocket receives a pong frame, which might include a payload
     */
    void webSocketPong(const QByteArray &payload, Context *c);

    /*!
     * Emitted when the websocket receives a close frame, including a close code and a reason
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

inline QStringList Request::args() const
{ return arguments(); }

inline QString Request::bodyParameter(const QString &key, const QString &defaultValue) const
{ return bodyParameters().value(key, defaultValue); }

inline QStringList Request::bodyParameters(const QString &key) const
{ return bodyParameters().values(key); }

inline ParamsMultiMap Request::bodyParams() const
{ return bodyParameters(); }

inline QString Request::bodyParam(const QString &key, const QString &defaultValue) const
{ return bodyParameters().value(key, defaultValue); }

inline QStringList Request::bodyParams(const QString &key) const
{ return bodyParameters().values(key); }

inline QString Request::queryParameter(const QString &key, const QString &defaultValue) const
{ return queryParameters().value(key, defaultValue); }

inline QStringList Request::queryParameters(const QString &key) const
{ return queryParameters().values(key); }

inline ParamsMultiMap Request::queryParams() const
{ return queryParameters(); }

inline QString Request::queryParam(const QString &key, const QString &defaultValue) const
{ return queryParameters().value(key, defaultValue); }

inline QStringList Request::queryParams(const QString &key) const
{ return queryParameters().values(key); }

inline QString Request::contentEncoding() const
{ return headers().contentEncoding(); }

inline QString Request::contentType() const
{ return headers().contentType(); }

inline QString Request::header(const QString &key) const
{ return headers().header(key); }

inline QString Request::userAgent() const
{ return headers().userAgent(); }

inline QString Request::referer() const
{ return headers().referer(); }

inline Upload *Request::upload(const QString &name) const
{ return uploadsMap().value(name); }

}

#endif // CUTELYST_REQUEST_H
