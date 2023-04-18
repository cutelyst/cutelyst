/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
    Q_PROPERTY(bool async READ async WRITE setAsync NOTIFY changed)
public:
    /**  This value defines which kind of connection should be used */
    enum ConnectionType {
        TcpConnection,
        SslConnection,
        TlsConnection,
    };
    Q_ENUM(ConnectionType)

    /**  This value defines which kind of authentication should be used */
    enum AuthMethod {
        AuthNone,
        AuthPlain,
        AuthLogin,
        AuthCramMd5,
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
     * Returns true if async mode is on.
     */
    bool async() const;

    /**
     * Enable sending mails in async mode, it will use SimpleMail::Server class,
     * and render() will always return true regardless of mail sending success.
     */
    void setAsync(bool enable);

    /**
     * Renders the EMail
     */
    virtual QByteArray render(Context *c) const override;

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

} // namespace Cutelyst

#endif // VIEWEMAIL_H
