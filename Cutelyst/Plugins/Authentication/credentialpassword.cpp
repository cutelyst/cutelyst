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

#include "credentialpassword.h"

#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALPASSWORD, "cutelyst.plugin.credentialpassword")

Authentication::User CredentialPassword::authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Authentication::User user = realm->findUser(ctx, authinfo);
    if (!user.isNull()) {
        if (checkPassword(user, authinfo)) {
            return user;
        }
        qCDebug(C_CREDENTIALPASSWORD) << "Password didn't match";
    } else {
        qCDebug(C_CREDENTIALPASSWORD) << "Unable to locate a user matching user info provided in realm";
    }
    return Authentication::User();
}

QString CredentialPassword::passwordField() const
{
    return m_passwordField;
}

void CredentialPassword::setPasswordField(const QString &fieldName)
{
    m_passwordField = fieldName;
}

CredentialPassword::Type CredentialPassword::passwordType() const
{
    return m_passwordType;
}

void CredentialPassword::setPasswordType(CredentialPassword::Type type)
{
    m_passwordType = type;
}

QCryptographicHash::Algorithm CredentialPassword::hashType() const
{
    return m_hashType;
}

void CredentialPassword::setHashType(QCryptographicHash::Algorithm type)
{
    m_hashType = type;
}

QString CredentialPassword::passwordPreSalt() const
{
    return m_passwordPreSalt;
}

void CredentialPassword::setPasswordPreSalt(const QString &passwordPreSalt)
{
    m_passwordPreSalt = passwordPreSalt;
}

QString CredentialPassword::passwordPostSalt() const
{
    return m_passwordPostSalt;
}

void CredentialPassword::setPasswordPostSalt(const QString &passwordPostSalt)
{
    m_passwordPostSalt = passwordPostSalt;
}

bool CredentialPassword::checkPassword(const Authentication::User &user, const CStringHash &authinfo)
{
    QString password = authinfo.value(m_passwordField);
    QString storedPassword = user.value(m_passwordField);

    if (m_passwordType == None) {
        qCDebug(C_CREDENTIALPASSWORD) << "CredentialPassword is set to ignore password check";
        return true;
    } else if (m_passwordType == Clear) {
        return storedPassword == password;
    } else if (m_passwordType == Hashed) {
        QCryptographicHash hash(m_hashType);
        hash.addData(m_passwordPreSalt.toUtf8());
        hash.addData(password.toUtf8());
        hash.addData(m_passwordPostSalt.toUtf8());
        QByteArray result =  hash.result();

        return storedPassword == result.toHex() ||
                storedPassword == result.toBase64();
    } else if (m_passwordType == SelfCheck) {
        return user.checkPassword(password);
    }

    return false;
}
