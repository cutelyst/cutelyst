/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CREDENTIALHTTP_P_H
#define CREDENTIALHTTP_P_H

#include "credentialhttp.h"

using namespace Qt::StringLiterals;

namespace Cutelyst {

class CredentialHttpPrivate
{
public:
    bool checkPassword(const AuthenticationUser &user, const ParamsMultiMap &authinfo);
    AuthenticationUser
        authenticateBasic(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo);
    AuthenticationUser authenticationFailed(Context *c,
                                            const AuthenticationRealm *realm,
                                            const ParamsMultiMap &authinfo);

    bool isAuthTypeDigest() const;
    bool isAuthTypeBasic() const;

    void createBasicAuthResponse(const Cutelyst::Context *c, const AuthenticationRealm *realm);
    QByteArrayList buildAuthHeaderCommon(const AuthenticationRealm *realm) const;
    QByteArray joinAuthHeaderParts(const QByteArray &type, const QByteArrayList &parts) const;

    CredentialHttp::AuthType type             = CredentialHttp::Any;
    CredentialHttp::PasswordType passwordType = CredentialHttp::None;
    QString usernameField                     = u"username"_s;
    QString passwordField                     = u"password"_s;
    QString passwordPreSalt;
    QString passwordPostSalt;
    QString authorizationRequiredMessage;
    bool requireSsl = false;
};

} // namespace Cutelyst

#endif // CREDENTIALHTTP_P_H
