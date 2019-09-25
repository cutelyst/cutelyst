/*
 * Copyright (C) 2015-2018 Daniel Nicoletti <dantti12@gmail.com>
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
    Q_DECLARE_PRIVATE(ViewEmail)
    Q_PROPERTY(QString stashKey READ stashKey WRITE setStashKey NOTIFY changed)
    Q_PROPERTY(QByteArray defaultContentType READ defaultContentType WRITE setDefaultContentType NOTIFY changed)
    Q_PROPERTY(QByteArray defaultCharset READ defaultCharset WRITE setDefaultCharset NOTIFY changed)
    Q_PROPERTY(QByteArray defaultEncoding READ defaultEncoding WRITE setDefaultEncoding NOTIFY changed)
public:
    /**  This value defines which kind of connection should be used */
    enum ConnectionType
    {
        TcpConnection,
        SslConnection,
        TlsConnection
    };
    Q_ENUM(ConnectionType)

    /**  This value defines which kind of authentication should be used */
    enum AuthMethod
    {
        AuthNone,
        AuthPlain,
        AuthLogin
    };
    Q_ENUM(AuthMethod)

    /*!
     * Constructs a new ViewEmail object with the given \p parent and \p name.
     */
    explicit ViewEmail(QObject *parent, const QString &name = QString());

    /**
     * Returns the stash key that will contain the email data
     */
    QString stashKey() const;

    /**
     * Defines the stash key that will contain the email data
     */
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

    /**
     * Returns the default encoding set
     */
    QByteArray defaultEncoding() const;

    /**
     * Defines the default encoding to be used when sending mails
     */
    void setDefaultEncoding(const QByteArray &encoding);

    /**
     * Returns the hostname of the SMTP server
     */
    QString senderHost() const;

    /**
     * Defines the hostname of the SMTP server
     */
    void setSenderHost(const QString &host);

    /**
     * Returns the port of the SMTP server
     */
    int senderPort() const;

    /**
     * Defines the port of the SMTP server
     */
    void setSenderPort(int port);

    /**
     * Defines the connection type of the SMTP server
     */
    ConnectionType senderConnectionType() const;

    /**
     * Returns the username that will authenticate on the SMTP server
     */
    void setSenderConnectionType(ConnectionType ct);

    /**
     * Returns the authenticaion method of the SMTP server
     */
    AuthMethod senderAuthMethod() const;

    /**
     * Defines the authenticaion method of the SMTP server
     */
    void setSenderAuthMethod(AuthMethod method);

    /**
     * Returns the username that will authenticate on the SMTP server
     */
    QString senderUser() const;

    /**
     * Defines the username that will authenticate on the SMTP server
     */
    void setSenderUser(const QString &user);

    /**
     * Returns the password that will authenticate on the SMTP server
     */
    QString senderPassword() const;

    /**
     * Defines the password that will authenticate on the SMTP server
     */
    void setSenderPassword(const QString &password);

    /**
     * Renders the EMail
     */
    QByteArray render(Context *c) const override;

protected:
    /*!
     * Constructs a new ViewEmail object using the private class, \p parent and \p name.
     */
    ViewEmail(ViewEmailPrivate *d, QObject *parent, const QString &name = QString());

Q_SIGNALS:
    void changed();

private:
    void initSender();
};

}

#endif // VIEWEMAIL_H
