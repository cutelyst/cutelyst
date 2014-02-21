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
#include <QFile>

namespace Cutelyst {

class Engine;
class RequestPrivate;
class Request
{
    Q_GADGET
public:
    ~Request();

    /**
     * @brief peerAddress
     * @return the address of the client
     */
    QHostAddress address() const;

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
    QByteArray base() const;

    /**
     * @brief path
     * @return the path, i.e. the part of the URI after base(), for the current request.
     * for  http://localhost/path/foo
     * path will contain 'path/foo'
     */
    QString path() const;
    QStringList args() const;
    QByteArray body() const;

    /**
     * Returns a QMultiHash of body (POST) parameters
     */
    QMultiHash<QString, QString> bodyParameters() const;

    /**
     * Short for \sa bodyParameters()
     */
    QMultiHash<QString, QString> bodyParam() const;

    /**
     * Returns a QMultiHash containing the query string (GET) parameters
     */
    QMultiHash<QString, QString> queryParameters() const;

    /**
     * Short for \sa queryParameters()
     */
    QMultiHash<QString, QString> queryParam() const;

    /**
     * Returns a QMultiHash containing both the query parameters (GET)
     * and the body parameters (POST)
     */
    QMultiHash<QString, QString> parameters() const;

    /**
     * Short for \sa parameters()
     */
    QMultiHash<QString, QString> param() const;


    QString contentEncoding() const;

    /**
     * @brief contentType
     * @return
     */
    QByteArray contentType() const;

    /**
     * Returns the cookie with the given name
     */
    QNetworkCookie cookie(const QByteArray &name) const;

    /**
     * Returns all the cookie from the request
     */
    QList<QNetworkCookie> cookies() const;
    QByteArray header(const QByteArray &key) const;
    QHash<QByteArray, QByteArray> headers() const;

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
    QByteArray userAgent() const;

    /**
     * @brief referer Shortcut for header("Referer")
     * @return the referring page.
     */
    QByteArray referer() const;

    /**
     * @brief remoteUser
     * @return the value of the REMOTE_USER environment variable.
     */
    QByteArray remoteUser() const;

    /**
     * @brief upload
     * @return a QFile pointer to the uploaded content, if any
     */
    QFile *upload() const;

    Engine *engine() const;

protected:
    Request(RequestPrivate *prv);
    void setArgs(const QStringList &args);

    RequestPrivate *d_ptr;

private:
    friend class Dispatcher;
    friend class Engine;
    Q_DECLARE_PRIVATE(Request)
};

}

#endif // CUTELYST_REQUEST_H
