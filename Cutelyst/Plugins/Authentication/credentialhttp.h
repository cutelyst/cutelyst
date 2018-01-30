/*
 * Copyright (C) 2013-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CREDENTIALHTTP_H
#define CREDENTIALHTTP_H

#include <QtCore/QCryptographicHash>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>

namespace Cutelyst {

class CredentialHttpPrivate;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT CredentialHttp : public AuthenticationCredential
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CredentialHttp)
public:
    enum PasswordType {
        None,
        Clear,
        Hashed
    };
    Q_ENUM(PasswordType)

    enum AuthType {
        Any,
        Basic
    };
    Q_ENUM(AuthType)

    /*!
     * Constructs a new CredentialHttp object with the given parent.
     */
    explicit CredentialHttp(QObject *parent = nullptr);
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

    /*!
     * Returns the field to look for when authenticating the user. \sa authenticate().
     */
    QString usernameField() const;

    /*!
     * Sets the field to look for when authenticating the user. \sa authenticate().
     */
    void setUsernameField(const QString &fieldName);

    /*!
     * Returns the field to look for when authenticating the user. \sa authenticate().
     */
    QString passwordField() const;

    /*!
     * Sets the field to look for when authenticating the user. \sa authenticate().
     */
    void setPasswordField(const QString &fieldName);

    /*!
     * Returns the type of password this class will be dealing with.
     */
    PasswordType passwordType() const;

    /*!
     * Sets the type of password this class will be dealing with.
     */
    void setPasswordType(PasswordType type);

    /*!
     * Returns the salt string to be prepended to the password
     */
    QString passwordPreSalt() const;

    /*!
     * Sets the salt string to be prepended to the password
     */
    void setPasswordPreSalt(const QString &passwordPreSalt);

    /*!
     * Returns the salt string to be appended to the password
     */
    QString passwordPostSalt() const;

    /*!
     * Sets the salt string to be appended to the password
     */
    void setPasswordPostSalt(const QString &passwordPostSalt);

    /**
     * If this configuration is true then authentication
     * will be denied (and a 401 issued in normal circumstances)
     * unless the request is via https.
     */
    void setRequireSsl(bool require);

    AuthenticationUser authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) final;

protected:
    CredentialHttpPrivate *d_ptr;

};

}

#endif // CREDENTIALHTTP_H
