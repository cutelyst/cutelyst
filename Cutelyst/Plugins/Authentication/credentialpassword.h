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

#include <QtCore/QCryptographicHash>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>

namespace Cutelyst {

class CredentialPasswordPrivate;
class CUTELYST_LIBRARY CredentialPassword : public AuthenticationCredential
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CredentialPassword)
public:
    enum Type {
        None,
        Clear,
        Hashed,
        SelfCheck
    };
    explicit CredentialPassword(QObject *parent = nullptr);
    virtual ~CredentialPassword();

    AuthenticationUser authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) Q_DECL_FINAL;

    QString passwordField() const;
    void setPasswordField(const QString &fieldName);

    Type passwordType() const;
    void setPasswordType(Type type);

    QString passwordPreSalt() const;
    void setPasswordPreSalt(const QString &passwordPreSalt);

    QString passwordPostSalt() const;
    void setPasswordPostSalt(const QString &passwordPostSalt);

    static bool validatePassword(const QByteArray &password, const QByteArray &correctHash);
    static QByteArray createPassword(const QByteArray &password, QCryptographicHash::Algorithm method, int iterations, int saltByteSize, int hashByteSize);
    static QByteArray pbkdf2(QCryptographicHash::Algorithm method,
                             const QByteArray &password, const QByteArray &salt,
                             int rounds, int keyLength);
    QByteArray hmac(QCryptographicHash::Algorithm method, QByteArray key, const QByteArray& message);

protected:
    CredentialPasswordPrivate *d_ptr;
};

} // namespace Plugin

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
