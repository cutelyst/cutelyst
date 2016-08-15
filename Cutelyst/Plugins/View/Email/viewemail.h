/*
 * Copyright (C) 2015-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef VIEWEMAIL_H
#define VIEWEMAIL_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewEmailPrivate;
/**
 * ViewEmail is a Cutelyst::View handler that sends stash
 * data via e-mail.
 */
class CUTELYST_VIEW_EMAIL_EXPORT ViewEmail : public Cutelyst::View
{
    Q_OBJECT
    Q_PROPERTY(QString stashKey READ stashKey WRITE setStashKey)
    Q_PROPERTY(QByteArray defaultContentType READ defaultContentType WRITE setDefaultContentType)
    Q_PROPERTY(QByteArray defaultCharset READ defaultCharset WRITE setDefaultCharset)
    Q_PROPERTY(QByteArray defaultEncoding READ defaultEncoding WRITE setDefaultEncoding)
public:
    explicit ViewEmail(QObject *parent, const QString &name = QString());
    virtual ~ViewEmail();

    QString stashKey() const;
    void setStashKey(const QString &stashKey);

    /**
     * Returns the default content type (mime type).
     */
    QByteArray defaultContentType() const;

    /**
     * Defines the default content type (mime type).
     */
    void setDefaultContentType(const QByteArray &contentType);

    /**
     * Returns the default charset for every MIME part with the
     * content type text.
     */
    QByteArray defaultCharset() const;

    /**
     * Defines the default charset for every MIME part with the
     * content type text.
     * According to RFC2049 a MIME part without a charset should
     * be treated as US-ASCII by the mail client.
     * If the charset is not set it won't be set for all MIME parts
     * without an overridden one.
     */
    void setDefaultCharset(const QByteArray &charset);

    QByteArray defaultEncoding() const;
    void setDefaultEncoding(const QByteArray &encoding);

    QString senderHost() const;
    void setSenderHost(const QString &host);

    int senderPort() const;
    void setSenderPort(int port);

    QString senderUser() const;
    void setSenderUser(const QString &user);

    QString senderPassword() const;
    void setSenderPassword(const QString &password);

    QByteArray render(Context *c) const Q_DECL_OVERRIDE;

protected:
    ViewEmail(ViewEmailPrivate *d, QObject *parent, const QString &name = QString());

    Q_DECLARE_PRIVATE(ViewEmail)
    ViewEmailPrivate *d_ptr;

private:
    void initSender();
};

}

#endif // VIEWEMAIL_H
