/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QCryptographicHash>

namespace Cutelyst {

class CredentialHttpPrivate;

/**
 * \ingroup plugins-authentication
 * \headerfile credentialhttp.h <Cutelyst/Plugins/Authentication/credentialhttp.h>
 * \brief Use HTTP basic authentication to authenticate a user.
 *
 * This credential provider authenticates a user using HTTP basic authentication as
 * described in <A HREF="https://datatracker.ietf.org/doc/html/rfc7617">RFC 76147</A>.
 * It tries to read the user name and the password from the \c Authorization header
 * send by the user agent. If the authorization fails or if no \c Authorization header
 * is available, it will respond with a <TT>401 Unauthorized</TT> status code and will
 * set the <TT>WWW-Authenticate</TT> header requesting basic authentication with the
 * used \link AuthenticationReam realm\endlink.
 *
 * For an example implementation see \ref plugins-authentication overview.
 *
 * \logcat{plugin.credentialhttp}
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT CredentialHttp : public AuthenticationCredential
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CredentialHttp)
public:
    /**
     * The used password type.
     */
    enum PasswordType {
        None,  /**< Ignore password check. */
        Clear, /**< Clear text password. */
        Hashed /**< Derived password hash using
                    <A HREF="https://datatracker.ietf.org/doc/html/rfc8018">PBKDF2</A> method. */
    };
    Q_ENUM(PasswordType)

    /**
     * The authentication type.
     */
    enum AuthType {
        Any,
        Basic,
    };
    Q_ENUM(AuthType)

    /**
     * Constructs a new %CredentialHttp object with the given \a parent.
     */
    explicit CredentialHttp(QObject *parent = nullptr);

    /**
     * Destroys the %CredentialHttp object.
     */
    virtual ~CredentialHttp();

    /**
     * Can be either any (the default), basic.
     *
     * This controls authorization_required_response and
     * authenticate, but not the "manual" methods.
     */
    void setType(CredentialHttp::AuthType type);

    /**
     * Set this to a string to override the default body content
     * "Authorization required.", or set to undef to suppress
     * body content being generated.
     */
    void setAuthorizationRequiredMessage(const QString &message);

    /**
     * Returns the field to look for when authenticating the user. \sa authenticate().
     */
    [[nodiscard]] QString usernameField() const;

    /**
     * Sets the field to look for when authenticating the user. \sa authenticate().
     */
    void setUsernameField(const QString &fieldName);

    /**
     * Returns the field to look for when authenticating the user. \sa authenticate().
     */
    [[nodiscard]] QString passwordField() const;

    /**
     * Sets the field to look for when authenticating the user. \sa authenticate().
     */
    void setPasswordField(const QString &fieldName);

    /**
     * Returns the type of password this class will be dealing with.
     */
    [[nodiscard]] PasswordType passwordType() const;

    /**
     * Sets the type of password this class will be dealing with.
     */
    void setPasswordType(PasswordType type);

    /**
     * Returns the salt string to be prepended to the password
     */
    [[nodiscard]] QString passwordPreSalt() const;

    /**
     * Sets the salt string to be prepended to the password
     */
    void setPasswordPreSalt(const QString &passwordPreSalt);

    /**
     * Returns the salt string to be appended to the password
     */
    [[nodiscard]] QString passwordPostSalt() const;

    /**
     * Sets the salt string to be appended to the password
     */
    void setPasswordPostSalt(const QString &passwordPostSalt);

    /**
     * If this configuration is true then authentication
     * will be denied (and a 401 issued in normal circumstances)
     * unless the request is via https.
     */
    void setRequireSsl(bool require);

    /**
     * Gets the user data from the \c Authorization HTTP header field and tries to find it in the
     * \a realm. On success, this returns a not null AuthenticationUser object. If authentication
     * fails, the HTTP response status code will be set to <TT>401 Unauthorized</TT> and the
     * <TT>WWW-Authenticate</TT> header will be set with the required authentication method and
     * \a realm name while a null AuthenticationUser object is returned.
     */
    [[nodiscard]] AuthenticationUser
        authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) final;

protected:
    CredentialHttpPrivate *d_ptr;
};

} // namespace Cutelyst
