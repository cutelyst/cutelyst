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

#include "credentialhttp.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QUrl>
#include <QStringBuilder>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALHTTP, "cutelyst.plugin.credentialhttp")

CredentialHttp::CredentialHttp()
{
}

void CredentialHttp::setType(CredentialHttp::AuthType type)
{
    m_type = type;
}

void CredentialHttp::setAuthorizationRequiredMessage(const QString &message)
{
    m_authorizationRequiredMessage = message;
}

QString CredentialHttp::passwordField() const
{
    return m_passwordField;
}

void CredentialHttp::setPasswordField(const QString &fieldName)
{
    m_passwordField = fieldName;
}

CredentialHttp::PasswordType CredentialHttp::passwordType() const
{
    return m_passwordType;
}

void CredentialHttp::setPasswordType(CredentialHttp::PasswordType type)
{
    m_passwordType = type;
}

QCryptographicHash::Algorithm CredentialHttp::hashType() const
{
    return m_hashType;
}

void CredentialHttp::setHashType(QCryptographicHash::Algorithm type)
{
    m_hashType = type;
}

QString CredentialHttp::passwordPreSalt() const
{
    return m_passwordPreSalt;
}

void CredentialHttp::setPasswordPreSalt(const QString &passwordPreSalt)
{
    m_passwordPreSalt = passwordPreSalt;
}

QString CredentialHttp::passwordPostSalt() const
{
    return m_passwordPostSalt;
}

void CredentialHttp::setPasswordPostSalt(const QString &passwordPostSalt)
{
    m_passwordPostSalt = passwordPostSalt;
}

QString CredentialHttp::usernameField() const
{
    return m_usernameField;
}

void CredentialHttp::setUsernameField(const QString &fieldName)
{
    m_usernameField = fieldName;
}

void CredentialHttp::setRequireSsl(bool require)
{
    m_requireSsl = require;
}

Authentication::User CredentialHttp::authenticate(Cutelyst::Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Authentication::User ret;
    if (m_requireSsl && !ctx->request()->secure()) {
        return authenticationFailed(ctx, realm, authinfo);
    }

    if (isAuthTypeDigest()) {
        ret = authenticateDigest(ctx, realm, authinfo);
        if (!ret.isNull()) {
            return ret;
        }
    }

    if (isAuthTypeBasic()) {
        ret = authenticateBasic(ctx, realm, authinfo);
        if (!ret.isNull()) {
            return ret;
        }
    }

    return authenticationFailed(ctx, realm, authinfo);
}

Authentication::User CredentialHttp::authenticateDigest(Cutelyst::Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    qCDebug(C_CREDENTIALHTTP) << "Checking http digest authentication.";

    return Authentication::User();
}

Authentication::User CredentialHttp::authenticateBasic(Cutelyst::Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Q_UNUSED(authinfo)
    qCDebug(C_CREDENTIALHTTP) << "Checking http basic authentication.";

    QPair<QString, QString> userPass = ctx->req()->headers().authorizationBasicPair();
    if (userPass.first.isEmpty()) {
        return Authentication::User();
    }

    CStringHash auth;
    auth.insert(m_usernameField, userPass.first);
    Authentication::User user = realm->findUser(ctx, auth);
    if (!user.isNull()) {
        auth.insert(m_passwordField, userPass.second);
        if (checkPassword(user, auth)) {
            return user;
        }
        qCDebug(C_CREDENTIALHTTP) << "Password didn't match";
    } else {
        qCDebug(C_CREDENTIALHTTP) << "Unable to locate a user matching user info provided in realm";
    }
    return Authentication::User();
}

Authentication::User CredentialHttp::authenticationFailed(Cutelyst::Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Response *res = ctx->response();
    res->setStatus(Response::Unauthorized); // 401
    res->setContentType(QByteArrayLiteral("text/plain; charset=UTF-8"));

    if (m_authorizationRequiredMessage.isNull()) {
        res->body() = QByteArrayLiteral("Authorization required.");
    } else {
        res->body() = m_authorizationRequiredMessage.toUtf8();
    }

    // Create Digest response
    if (isAuthTypeDigest()) {
//        _create_digest_auth_response TODO
    }

    // Create Basic response
    if (isAuthTypeBasic()) {
        createBasicAuthResponse(ctx);
    }

    return Authentication::User();
}

bool CredentialHttp::checkPassword(const Authentication::User &user, const CStringHash &authinfo)
{
    QString password = authinfo.value(m_passwordField);
    QString storedPassword = user.value(m_passwordField);

    if (m_passwordType == None) {
        qCDebug(C_CREDENTIALHTTP) << "CredentialPassword is set to ignore password check";
        return true;
    } else if (m_passwordType == Clear) {
        return storedPassword == password;
    } else if (m_passwordType == Hashed) {
        QCryptographicHash hash(m_hashType);
        hash.addData(m_passwordPreSalt.toUtf8());
        hash.addData(password.toUtf8());
        hash.addData(m_passwordPostSalt.toUtf8());
        QByteArray result =  hash.result();

        return storedPassword == result ||
                storedPassword == result.toHex() ||
                storedPassword == result.toBase64();
    } else if (m_passwordType == SelfCheck) {
        return user.checkPassword(password);
    }

    return false;
}

bool CredentialHttp::isAuthTypeDigest() const
{
    return m_type == AuthType::Digest || m_type == AuthType::Any;
}

bool CredentialHttp::isAuthTypeBasic() const
{
    return m_type == AuthType::Basic || m_type == AuthType::Any;
}

QStringList CredentialHttp::buildAuthHeaderCommon() const
{
    // TODO
    // return realm="realmname"
    // return domain="realmname"
    return QStringList();
}

QString CredentialHttp::joinAuthHeaderParts(const QString &type, const QStringList &parts) const
{
    if (parts.isEmpty()) {
        return type;
    } else {
        return type % QLatin1Char(' ') % parts.join(QStringLiteral(", "));
    }
}

void CredentialHttp::createBasicAuthResponse(Cutelyst::Context *ctx)
{
    ctx->res()->headers().setWwwAuthenticate(joinAuthHeaderParts(QStringLiteral("Basic"),
                                                                 buildAuthHeaderCommon()).toLatin1());
}
