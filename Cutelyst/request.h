/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_REQUEST_H
#define CUTELYST_REQUEST_H

#include <QObject>
#include <QHostAddress>
#include <QNetworkCookie>

#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Headers>
typedef QMap<QString, QString> Params;

namespace Cutelyst {

class Engine;
class Upload;

typedef QList<Upload *> Uploads;

class RequestPrivate;
class Request : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname)
    Q_PROPERTY(quint16 port READ port)
    Q_PROPERTY(QUrl uri READ uri)
    Q_PROPERTY(QString base READ base)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString match READ match)
    Q_PROPERTY(QStringList arguments READ arguments)
    Q_PROPERTY(QStringList args READ args)
    Q_PROPERTY(bool secure READ secure)
    Q_PROPERTY(QVariant bodyData READ bodyData)
    Q_PROPERTY(Cutelyst::ParamsMultiMap bodyParams READ bodyParameters)
    Q_PROPERTY(Cutelyst::ParamsMultiMap queryParams READ queryParameters)
    Q_PROPERTY(Cutelyst::ParamsMultiMap parameters READ parameters)
    Q_PROPERTY(Cutelyst::ParamsMultiMap params READ parameters)
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
    QStringList args() const;

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
     * Returns a QMultiHash of body (POST) parameters
     */
    ParamsMultiMap bodyParameters() const;

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    inline QString bodyParameter(const QString &key, const QString &defaultValue = QString()) const
    { return bodyParameters().value(key, defaultValue); }

    /**
     * Convenience method for geting all body values passing a key
     */
    inline QStringList bodyParameters(const QString &key) const
    { return bodyParameters().values(key); }

    /**
     * Short for bodyParameters()
     */
    inline ParamsMultiMap bodyParams() const
    { return bodyParameters(); }

    /**
     * Convenience method for geting a single body value passing a key and an optional default value
     */
    inline QString bodyParam(const QString &key, const QString &defaultValue = QString()) const
    { return bodyParameters().value(key, defaultValue); }

    /**
     * Convenience method for geting all body values passing a key
     */
    inline QStringList bodyParams(const QString &key) const
    { return bodyParameters().values(key); }

    /**
     * Contains the keywords portion of a query string, when no '=' signs are present.
     * * \code
     * http://localhost/path?some+keywords
     * c->request()->queryKeywords() will contain 'some keywords'
     * \endcode
     */
    QString queryKeywords() const;

    /**
     * Returns a QMultiHash containing the query string (GET) parameters
     */
    ParamsMultiMap queryParameters() const;

    /**
     * Convenience method for geting a single query value passing a key and an optional default value
     */
    inline QString queryParameter(const QString &key, const QString &defaultValue = QString()) const
    { return queryParameters().value(key, defaultValue); }

    /**
     * Convenience method for geting all query values passing a key
     */
    inline QStringList queryParameters(const QString &key) const
    { return queryParameters().values(key); }

    /**
     * Short for queryParameters()
     */
    inline ParamsMultiMap queryParams() const
    { return queryParameters(); }

    /**
     * Convenience method for geting a single query value passing a key and an optional default value
     */
    inline QString queryParam(const QString &key, const QString &defaultValue = QString()) const
    { return queryParameters().value(key, defaultValue); }

    /**
     * Convenience method for geting all query values passing a key
     */
    inline QStringList queryParams(const QString &key) const
    { return queryParameters().values(key); }

    /**
     * Returns a QMultiHash containing both the query parameters (GET)
     * and the body parameters (POST)
     */
    ParamsMultiMap parameters() const;

    /**
     * Returns the value specified by key, it's equivalent to calling
     * parameters().value().
     */
    inline QString param(const QString &key, const QString &defaultValue = QString()) const
    { return parameters().value(key, defaultValue); }

    /**
     * Returns the values specified by key, it's equivalent to calling
     * parameters().values().
     */
    inline QStringList params(const QString &key) const
    { return parameters().values(key); }

    /**
     * Short for parameters()
     */
    inline ParamsMultiMap params() const { return parameters(); }

    /**
     * Returns the Content-Encoding header
     */
    inline QString contentEncoding() const { return headers().contentEncoding(); }

    /**
     * Returns the Content-Type header
     */
    inline QString contentType() const { return headers().contentType(); }

    /**
     * Returns the cookie with the given name
     */
    QNetworkCookie cookie(const QString &name) const;

    /**
     * Returns all the cookie from the request
     */
    QList<QNetworkCookie> cookies() const;

    /**
     * Short for headers().header(key);
     */
    inline QString header(const QString &key) const { return headers().header(key); }

    /**
     * Returns the HTTP request headers
     */
    Headers headers() const;

    /**
     * Returns the request method (GET, POST, HEAD, etc).
     */
    QString method() const;

    /**
     * Returns the protocol (HTTP/1.0 or HTTP/1.1) used for the current request.
     */
    QString protocol() const;

    /**
     * Returns the user agent (browser) version string.
     */
    QString userAgent() const { return headers().userAgent(); }

    /**
     * referer Shortcut for header("Referer")
     */
    QString referer() const { return headers().referer(); }

    /**
     * Returns the value of the REMOTE_USER environment variable.
     */
    QString remoteUser() const;

    QMap<QString, Upload *> uploads() const;

    inline Uploads uploads(const QString &name) const
    { return uploads().values(name); }

    inline Upload *upload(const QString &name) const
    { return uploads().value(name); }

    Engine *engine() const;

    void *engineData();

    Request(RequestPrivate *prv);

protected:
    RequestPrivate *d_ptr;

private:
    friend class Application;
    friend class Dispatcher;
    friend class DispatchType;
    Q_DECLARE_PRIVATE(Request)
};

}

#endif // CUTELYST_REQUEST_H
