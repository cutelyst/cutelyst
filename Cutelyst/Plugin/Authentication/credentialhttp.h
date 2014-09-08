/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CREDENTIALHTTP_H
#define CREDENTIALHTTP_H

#include <Cutelyst/Plugin/authentication.h>

namespace Cutelyst {
namespace Plugin {

class CredentialHttp : public Authentication::Credential
{
public:
    enum Type {
        Any,
        Digest,
        Basic
    };
    explicit CredentialHttp();

    /**
     * Can be either any (the default), basic or digest.
     *
     * This controls authorization_required_response and
     * authenticate, but not the "manual" methods.
     */
    void setType(CredentialHttp::Type type);

    /**
     * Set this to a string to override the default body content
     * "Authorization required.", or set to undef to suppress
     * body content being generated.
     */
    void setAuthorizationRequiredMessage(const QString &message);

    QString passwordField() const;
    void setPasswordField(const QString &fieldName);

    QString usernameField() const;
    void setUsernameField(const QString &fieldName);

    /**
     * If this configuration is true then authentication
     * will be denied (and a 401 issued in normal circumstances)
     * unless the request is via https.
     */
    void setRequireSsl(bool require);

    Authentication::User authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);

private:
    Authentication::User authenticateDigest(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);
    Authentication::User authenticateBasic(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);
    Authentication::User authenticationFailed(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);

    bool isAuthTypeDigest() const;
    bool isAuthTypeBasic() const;

    Type m_type = Type::Any;
    QString m_passwordField;
    QString m_usernameField = QStringLiteral("username");
    QString m_authorizationRequiredMessage;
    bool m_requireSsl = false;
};

}
}

#endif // CREDENTIALHTTP_H
