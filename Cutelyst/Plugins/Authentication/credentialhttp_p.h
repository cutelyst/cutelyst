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

#ifndef CREDENTIALHTTP_P_H
#define CREDENTIALHTTP_P_H

#include "credentialhttp.h"

namespace Cutelyst {

class CredentialHttpPrivate
{
public:
    bool checkPassword(const AuthenticationUser &user, const ParamsMultiMap &authinfo);
    AuthenticationUser authenticateBasic(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo);
    AuthenticationUser authenticationFailed(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo);

    bool isAuthTypeDigest() const;
    bool isAuthTypeBasic() const;

    void createBasicAuthResponse(Context *c);
    QStringList buildAuthHeaderCommon() const;
    QString joinAuthHeaderParts(const QString &type, const QStringList &parts) const;

    CredentialHttp::AuthType type = CredentialHttp::Any;
    QString usernameField = QStringLiteral("username");
    QString passwordField = QStringLiteral("password");
    CredentialHttp::PasswordType passwordType = CredentialHttp::None;
    QString passwordPreSalt;
    QString passwordPostSalt;
    QString authorizationRequiredMessage;
    bool requireSsl = false;
};

}

#endif // CREDENTIALHTTP_P_H

