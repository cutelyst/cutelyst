/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef HEADERS_H
#define HEADERS_H

#include <QMap>
#include <QDate>

namespace Cutelyst {

class HeadersPrivate;
class Headers : public QMap<QByteArray, QByteArray>
{
public:
    Headers();

    qint64 contentLength() const;

    void setContentLength(qint64 value);

    /**
     * This header represents the date and time at which the message was originated
     */
    QDateTime date() const;

    /**
     * Defines the header that represents the date and time at which the message was originated
     */
    void setDate(const QDateTime &date);

    /**
     * Returns the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    QByteArray server() const;

    /**
     * Defines the server header field contains information about the software
     * being used by the originating server program handling the request.
     */
    void setServer(const QByteArray &value);

    QByteArray header(const QByteArray &field) const;

    void setHeader(const QString &field, const QStringList &values);
    void setHeader(const QByteArray &field, const QByteArray &value);
};

}

#endif // HEADERS_H
