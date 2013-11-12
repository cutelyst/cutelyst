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

class CutelystResponsePrivate;
class CutelystResponse : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CutelystResponse)
    Q_ENUMS(HttpStatus)
public:
    enum HttpStatus {
        Ok                    = 200,
        MovedPermanently      = 301,
        Found                 = 302,
        NotModified           = 304,
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
    QString statusString() const;
    bool finalizedHeaders() const;
    QString redirect() const;

    void setHeaderValue(const QString &key, const QString &value);
    bool hasBody() const;
    QByteArray body() const;
    void setBody(const QByteArray &body);
    void setContentEncoding(const QString &encoding);
    void setContentLength(quint64 length);
    void setContentType(const QString &encoding);
    void setCookie(const QString &key, const QString &value);
    void setCookies(const QHash<QString, QString> &cookies);

    void setRedirect(const QString &url, HttpStatus status = Found);
    void setLocation(const QString &location);
    QMap<QString, QString> headers() const;
    void write(const QByteArray &data);

protected:
    CutelystResponsePrivate *d_ptr;
};

#endif // CUTELYSTRESPONSE_H
