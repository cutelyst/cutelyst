/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWEMAIL_H
#define VIEWEMAIL_H

#include <Cutelyst/Plugins/View/Email/cutelyst_view_email_export.h>
#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewEmailPrivate;
/**
 * \ingroup plugins-view
 * \headerfile "" <Cutelyst/Plugins/View/Email/viewemail.h>
 * \brief A view that sends stash data via e-mail.
 *
 * %ViewEmail is a View handler that sends Context::stash() data via e-mail.
 *
 * \logcat{view.email}
 */
class CUTELYST_VIEW_EMAIL_EXPORT ViewEmail : public Cutelyst::View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewEmail)
    /**
     * The stash key that will contain the email data.
     */
    Q_PROPERTY(QString stashKey READ stashKey WRITE setStashKey NOTIFY changed)
    /**
     * The default content type (mime type) that is used if there is no content type
     * set in the stash data. \c text/plain by default.
     */
    Q_PROPERTY(QByteArray defaultContentType READ defaultContentType WRITE setDefaultContentType
                   NOTIFY changed)
    /**
     * The default charset for every MIME part with the content type text that is used if there
     * is no charset set in the stash data.
     */
    Q_PROPERTY(QByteArray defaultCharset READ defaultCharset WRITE setDefaultCharset NOTIFY changed)
    /**
     * The default encoding that is used if there is no encoding set in the stash data.
     */
    Q_PROPERTY(
        QByteArray defaultEncoding READ defaultEncoding WRITE setDefaultEncoding NOTIFY changed)
public:
    /**  This value defines which kind of connection should be used */
    enum ConnectionType {
        TcpConnection, /**< Use an unsecured TCP connection. */
        SslConnection, /**< Use a SSL/TLS secured connection. */
        TlsConnection, /**< Use StartTLS to upgrade an unencrypted connection to use TLS. */
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

    /**
     * Constructs a new %ViewEmail object with the given \a parent and \a name.
     *
     * The \a name can be used to specify different views that can be called either dynamically
     * by Context::setCustomView() or with the \c :View() argument of the RenderView action.
     */
    explicit ViewEmail(QObject *parent, const QString &name = QString());

    /**
     * Returns the stash key that will contain the email data.
     * \sa setStashKey()
     */
    [[nodiscard]] QString stashKey() const;

    /**
     * Defines the \a stashKey that will contain the email data.
     * \sa stashKey()
     */
    void setStashKey(const QString &stashKey);

    /**
     * Returns the default content type (mime type) that is used if there is no content type
     * set in the stash data. \c text/plain by default.
     * \sa setDefaultContentType()
     */
    [[nodiscard]] QByteArray defaultContentType() const;

    /**
     * Sets the default \a contentType (mime type) that is used if there is no content type
     * set in the stash data.
     * \sa defaultContentType()
     */
    void setDefaultContentType(const QByteArray &contentType);

    /**
     * Returns the default charset for every MIME part with the
     * content type text that is used if there is no charset set in the stash data.
     * \sa setDefaultCharset()
     */
    [[nodiscard]] QByteArray defaultCharset() const;

    /**
     * Defines the default charset for every MIME part with the
     * content type text  that is used if there is no charset set in the stash data.
     * According to RFC2049 a MIME part without a charset should
     * be treated as US-ASCII by the mail client.
     * If the charset is not set it won't be set for all MIME parts
     * without an overridden one.
     * \sa defaultCharset()
     */
    void setDefaultCharset(const QByteArray &charset);

    /**
     * Returns the default encoding that is used if there is no encoding
     * set in the stash data.
     * \sa setDefaultEncoding()
     */
    [[nodiscard]] QByteArray defaultEncoding() const;

    /**
     * Defines the default encoding that is used if there is no encoding
     * set in the stash data.
     * \sa defaultEncoding()
     */
    void setDefaultEncoding(const QByteArray &encoding);

    /**
     * Returns the hostname of the SMTP server.
     * \sa setSenderHost()
     */
    [[nodiscard]] QString senderHost() const;

    /**
     * Defines the hostname of the SMTP server.
     * \sa senderHost()
     */
    void setSenderHost(const QString &host);

    /**
     * Returns the port of the SMTP server.
     * \sa setSenderPort()
     */
    [[nodiscard]] int senderPort() const;

    /**
     * Defines the port of the SMTP server.
     * \sa senderPort()
     */
    void setSenderPort(int port);

    /**
     * Returns the connection type of the SMTP server.
     * \sa setSenderConnectionType()
     */
    [[nodiscard]] ConnectionType senderConnectionType() const;

    /**
     * Defines the connection type of the SMTP server.
     * \sa senderConnectionType()
     */
    void setSenderConnectionType(ConnectionType ct);

    /**
     * Returns the authenticaion method of the SMTP server.
     * \sa setSenderAuthMethod()
     */
    [[nodiscard]] AuthMethod senderAuthMethod() const;

    /**
     * Defines the authenticaion method of the SMTP server.
     * \sa senderAuthMethod()
     */
    void setSenderAuthMethod(AuthMethod method);

    /**
     * Returns the username that will authenticate on the SMTP server.
     * \sa setSenderUser()
     */
    [[nodiscard]] QString senderUser() const;

    /**
     * Defines the username that will authenticate on the SMTP server.
     * \sa senderUser()
     */
    void setSenderUser(const QString &user);

    /**
     * Returns the password that will authenticate on the SMTP server.
     * \sa setSenderPassword()
     */
    [[nodiscard]] QString senderPassword() const;

    /**
     * Defines the password that will authenticate on the SMTP server.
     * \sa senderPassword()
     */
    void setSenderPassword(const QString &password);

    /**
     * Renders and sends the email. This will always return an emty byte array,
     * regardless of mail sending success.
     */
    QByteArray render(Context *c) const override;

protected:
    /**
     * Constructs a new %ViewEmail object using the private class \a d, \a parent and \a name.
     */
    ViewEmail(ViewEmailPrivate *d, QObject *parent, const QString &name = QString());

Q_SIGNALS:
    void changed();

private:
    void initSender();
};

} // namespace Cutelyst

#endif // VIEWEMAIL_H
