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

#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include <Cutelyst/Plugin/authentication.h>
#include <QCryptographicHash>

namespace Cutelyst {
namespace Plugin {

class CredentialPassword : public Authentication::Credential
{
public:
    enum Type {
        None,
        Clear,
        Hashed,
        SelfCheck
    };
    CredentialPassword();

    Authentication::User authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);

    QString passwordField() const;
    void setPasswordField(const QString &fieldName);

    Type passwordType() const;
    void setPasswordType(Type type);

    QCryptographicHash::Algorithm hashType() const;
    void setHashType(QCryptographicHash::Algorithm type);

    QString passwordPreSalt() const;
    void setPasswordPreSalt(const QString &passwordPreSalt);

    QString passwordPostSalt() const;
    void setPasswordPostSalt(const QString &passwordPostSalt);

private:
    bool checkPassword(const Authentication::User &user, const CStringHash &authinfo);

    QString m_passwordField;
    Type m_passwordType;
    QCryptographicHash::Algorithm m_hashType;
    QString m_passwordPreSalt;
    QString m_passwordPostSalt;
};

} // namespace Plugin
}

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
