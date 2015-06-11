/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "credentialpassword_p.h"
#include "../authenticationrealm.h"

#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALPASSWORD, "cutelyst.plugin.credentialpassword")

CredentialPassword::CredentialPassword(QObject *parent) : AuthenticationCredential(parent)
  , d_ptr(new CredentialPasswordPrivate)
{

}

CredentialPassword::~CredentialPassword()
{
    delete d_ptr;
}

AuthenticationUser CredentialPassword::authenticate(Context *c, AuthenticationRealm *realm, const CStringHash &authinfo)
{
    Q_D(CredentialPassword);
    AuthenticationUser user = realm->findUser(c, authinfo);
    if (!user.isNull()) {
        if (d->checkPassword(user, authinfo)) {
            return user;
        }
        qCDebug(C_CREDENTIALPASSWORD) << "Password didn't match";
    } else {
        qCDebug(C_CREDENTIALPASSWORD) << "Unable to locate a user matching user info provided in realm";
    }
    return AuthenticationUser();
}

QString CredentialPassword::passwordField() const
{
    Q_D(const CredentialPassword);
    return d->passwordField;
}

void CredentialPassword::setPasswordField(const QString &fieldName)
{
    Q_D(CredentialPassword);
    d->passwordField = fieldName;
}

CredentialPassword::Type CredentialPassword::passwordType() const
{
    Q_D(const CredentialPassword);
    return d->passwordType;
}

void CredentialPassword::setPasswordType(CredentialPassword::Type type)
{
    Q_D(CredentialPassword);
    d->passwordType = type;
}

QCryptographicHash::Algorithm CredentialPassword::hashType() const
{
    Q_D(const CredentialPassword);
    return d->hashType;
}

void CredentialPassword::setHashType(QCryptographicHash::Algorithm type)
{
    Q_D(CredentialPassword);
    d->hashType = type;
}

QString CredentialPassword::passwordPreSalt() const
{
    Q_D(const CredentialPassword);
    return d->passwordPreSalt;
}

void CredentialPassword::setPasswordPreSalt(const QString &passwordPreSalt)
{
    Q_D(CredentialPassword);
    d->passwordPreSalt = passwordPreSalt;
}

QString CredentialPassword::passwordPostSalt() const
{
    Q_D(const CredentialPassword);
    return d->passwordPostSalt;
}

void CredentialPassword::setPasswordPostSalt(const QString &passwordPostSalt)
{
    Q_D(CredentialPassword);
    d->passwordPostSalt = passwordPostSalt;
}

bool CredentialPasswordPrivate::checkPassword(const AuthenticationUser &user, const CStringHash &authinfo)
{
    QString password = authinfo.value(passwordField);
    QString storedPassword = user.value(passwordField);

    if (passwordType == CredentialPassword::None) {
        qCDebug(C_CREDENTIALPASSWORD) << "CredentialPassword is set to ignore password check";
        return true;
    } else if (passwordType == CredentialPassword::Clear) {
        return storedPassword == password;
    } else if (passwordType == CredentialPassword::Hashed) {
        QCryptographicHash hash(hashType);
        hash.addData(passwordPreSalt.toUtf8());
        hash.addData(password.toUtf8());
        hash.addData(passwordPostSalt.toUtf8());
        QByteArray result =  hash.result();

        return storedPassword == result.toHex() ||
                storedPassword == result.toBase64();
    } else if (passwordType == CredentialPassword::SelfCheck) {
        return user.checkPassword(password);
    }

    return false;
}
