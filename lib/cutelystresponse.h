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

#ifndef CUTELYSTRESPONSE_H
#define CUTELYSTRESPONSE_H

#include <QObject>
#include <QNetworkCookie>

class CutelystResponsePrivate;
class CutelystResponse : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CutelystResponse)
    Q_ENUMS(HttpStatus)
public:
    enum HttpStatus {
        OK                    = 200,
        MultipleChoices       = 300,
        MovedPermanently      = 301,
        Found                 = 302,
        SeeOther              = 303, // Since HTTP/1.1
        NotModified           = 304,
        UseProxy              = 305, // Since HTTP/1.1
        TemporaryRedirect     = 307, // Since HTTP/1.1
        PermanentRedirect     = 308, // Since HTTP/1.1
        BadRequest            = 400,
        AuthorizationRequired = 401,
        Forbidden             = 403,
        NotFound              = 404,
        MethodNotAllowed      = 405,
        InternalServerError   = 500
    };
    explicit CutelystResponse(QObject *parent = 0);
    ~CutelystResponse();

    quint16 status() const;
    void setStatus(quint16 status);
    bool finalizedHeaders() const;

    void addHeaderValue(const QString &key, const QByteArray &value);
    bool hasBody() const;
    QByteArray &body();
    void setContentEncoding(const QString &encoding);
    void setContentLength(quint64 length);
    void setContentType(const QString &encoding);
    QList<QNetworkCookie> cookies() const;
    void addCookie(const QNetworkCookie &cookie);
    void setCookies(const QList<QNetworkCookie> &cookies);

    /**
     * Causes the response to redirect to the specified URL. The default status is 302.
     * This is a convenience method that sets the Location header to the redirect
     * destination, and then sets the response status. You will want to return false or
     * c->detach() to interrupt the normal processing flow if you want the redirect to
     * occur straight away.
     */
    void redirect(const QString &url, quint16 status = Found);

    /**
     * Returns the HTTP location set by the redirect
     */
    QUrl location() const;

    QMap<QString, QString> headers() const;
    void write(const QByteArray &data);

protected:
    CutelystResponsePrivate *d_ptr;
};

#endif // CUTELYSTRESPONSE_H
