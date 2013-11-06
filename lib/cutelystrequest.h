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

#ifndef CUTELYSTREQUEST_H
#define CUTELYSTREQUEST_H

#include <QObject>
#include <QTcpSocket>

class CutelystRequestPrivate;
class CutelystRequest
{
public:
    CutelystRequest();
    CutelystRequest(CutelystRequestPrivate *prv);
    ~CutelystRequest();

    /**
     * @brief peerAddress
     * @return the address of the client
     */
    QHostAddress peerAddress() const;

    /**
     * @brief peerName
     * @return the hostname of the client
     */
    QString peerName() const;

    /**
     * @brief peerPort
     * @return the originating port of the client
     */
    quint16 peerPort() const;

    QString path() const;
    QStringList args() const;
    QString base() const;
    QString body() const;
    QVariantHash bodyParameters() const;
    QString contentEncoding() const;
    QString cookie(const QString &key) const;
    QHash<QString, QString> cookies() const;
    QString header(const QString &key) const;
    QHash<QString, QString> headers() const;
    QString method() const;
    QString protocol() const;
    QString userAgent() const;

protected:
    void setArgs(const QStringList &args);

    CutelystRequestPrivate *d_ptr;

private:
    friend class CutelystDispatcher;
    Q_DECLARE_PRIVATE(CutelystRequest)
};

#endif // CUTELYSTREQUEST_H
