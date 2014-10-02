/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

namespace Cutelyst {

class Engine;
class Upload;

typedef QList<Upload *> Uploads;

class RequestPrivate;
class Request
{
    Q_GADGET
public:
    virtual ~Request();

    /**
     * @brief peerAddress
     * @return the address of the client
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
     * @brief peerPort
     * @return the originating port of the client
     */
    quint16 port() const;

    /**
     * @brief uri
     * @return the uri as close as possible to what
     * the user has in his browser url.
     */
    QUrl uri() const;

    /**
     * @brief base
     * @return Contains the URI base. This will always have a trailing slash.
     * Note that the URI scheme (e.g., http vs. https) must be determined through
     * heuristics; depending on your server configuration, it may be incorrect.
     * See \sa secure() for more info.
     *
     * If your application was queried with the URI http://localhost:3000/some/path
     * then base is http://localhost:3000/.
     */
    QUrl base() const;

    /**
     * @brief path
     * @return the path, i.e. the part of the URI after base(), for the current request.
     * for  http://localhost/path/foo
     * path will contain '/path/foo'
     */
    QByteArray path() const;

    /**
     * This contains the matching part of a Regex action.
     * Otherwise it returns the same as 'action' (not a pointer but it's private name),
     * except for default actions, which return an empty string.
     */
    QByteArray match() const;

    QStringList args() const;

    /**
     * Returns true or false to indicate that a connection is secure (https),
     * The reliability of it may depend on your server configuration, Cutelyst
     * relies on the Engine to set this information which is used to build up
     * uri().scheme(). The Engine itself might not be aware of a front HTTP
     * server with https enabled.
     */
    bool secure() const;

    /**
     * @brief body
     * @return the message body of the request as
     * passed by the Engine, this can even be a file
     * if the Engine wants to.
     */
    QIODevice *body() const;

    /**
     * Returns a QMultiHash of body (POST) parameters
     */
    ParamsMultiMap bodyParameters() const;

    /**
     * Short for \sa bodyParameters()
     */
    inline ParamsMultiMap bodyParam() const { return bodyParameters(); }

    /**
     * Returns a QMultiHash containing the query string (GET) parameters
     */
    ParamsMultiMap queryParameters() const;

    /**
     * Short for \sa queryParameters()
     */
    inline ParamsMultiMap queryParam() const { return queryParameters(); }

    /**
     * Returns a QMultiHash containing both the query parameters (GET)
     * and the body parameters (POST)
     */
    ParamsMultiMap parameters() const;

    /**
     * Short for \sa parameters()
     */
    inline ParamsMultiMap param() const { return parameters(); }

    /**
     * @brief contentEncoding
     * @return the Content-Encoding header
     */
    inline QByteArray contentEncoding() const { return headers().contentEncoding(); }

    /**
     * @brief contentType
     * @return the Content-Type header
     */
    inline QByteArray contentType() const { return headers().contentType(); }

    /**
     * Returns the cookie with the given name
     */
    QNetworkCookie cookie(const QByteArray &name) const;

    /**
     * Returns all the cookie from the request
     */
    QList<QNetworkCookie> cookies() const;

    inline QByteArray header(const QByteArray &key) const { return headers().value(key); }

    Headers headers() const;

    /**
     * @brief method
     * @return the request method (GET, POST, HEAD, etc).
     */
    QByteArray method() const;

    /**
     * @brief protocol
     * @return the protocol (HTTP/1.0 or HTTP/1.1) used for the current request.
     */
    QByteArray protocol() const;

    /**
     * @brief userAgent
     * @return the user agent (browser) version string.
     */
    QByteArray userAgent() const { return headers().userAgent(); }

    /**
     * @brief referer Shortcut for header("Referer")
     * @return the referring page.
     */
    QByteArray referer() const { return headers().referer(); }

    /**
     * @brief remoteUser
     * @return the value of the REMOTE_USER environment variable.
     */
    QByteArray remoteUser() const;

    Uploads uploads() const;

    Engine *engine() const;

    void *engineData();

    Request(RequestPrivate *prv);

protected:
    void setArgs(const QStringList &args);

    RequestPrivate *d_ptr;

private:
    friend class Dispatcher;
    friend class DispatchType;
    Q_DECLARE_PRIVATE(Request)
};

}

#endif // CUTELYST_REQUEST_H
