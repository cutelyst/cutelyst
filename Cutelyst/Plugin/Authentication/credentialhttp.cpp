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
#include <QLoggingCategory>

using namespace Cutelyst::Plugin;

Q_LOGGING_CATEGORY(C_CREDENTIALHTTP, "cutelyst.plugin.credentialhttp")

CredentialHttp::CredentialHttp()
{
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
    if (m_requireSsl && ctx->request()->uri().scheme() != QLatin1String("https")) {
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

    Authentication::User user = realm->findUser(ctx, authinfo);
    if (!user.isNull()) {

    } else {
        qCDebug(C_CREDENTIALHTTP) << "Unable to locate a user matching user info provided in realm";
    }
    return Authentication::User();
}

Authentication::User CredentialHttp::authenticateBasic(Cutelyst::Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    qCDebug(C_CREDENTIALHTTP) << "Checking http basic authentication.";

    Authentication::User user = realm->findUser(ctx, authinfo);
    if (!user.isNull()) {

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
    if (isAuthTypeDigest()) {
//        _create_basic_auth_response TODO
    }

    return Authentication::User();
}

bool CredentialHttp::isAuthTypeDigest() const
{
    return m_type == Type::Digest || m_type == Type::Any;
}

bool CredentialHttp::isAuthTypeBasic() const
{
    return m_type == Type::Basic || m_type == Type::Any;
}
