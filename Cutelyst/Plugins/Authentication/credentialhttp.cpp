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

#include "credentialhttp_p.h"
#include "credentialpassword.h"

#include "authenticationrealm.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QUrl>
#include <QStringBuilder>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALHTTP, "cutelyst.plugin.credentialhttp")

CredentialHttp::CredentialHttp(QObject *parent) : AuthenticationCredential(parent)
  , d_ptr(new CredentialHttpPrivate)
{
}

CredentialHttp::~CredentialHttp()
{
    delete d_ptr;
}

void CredentialHttp::setType(CredentialHttp::AuthType type)
{
    Q_D(CredentialHttp);
    d->type = type;
}

void CredentialHttp::setAuthorizationRequiredMessage(const QString &message)
{
    Q_D(CredentialHttp);
    d->authorizationRequiredMessage = message;
}

QString CredentialHttp::passwordField() const
{
    Q_D(const CredentialHttp);
    return d->passwordField;
}

void CredentialHttp::setPasswordField(const QString &fieldName)
{
    Q_D(CredentialHttp);
    d->passwordField = fieldName;
}

CredentialHttp::PasswordType CredentialHttp::passwordType() const
{
    Q_D(const CredentialHttp);
    return d->passwordType;
}

void CredentialHttp::setPasswordType(CredentialHttp::PasswordType type)
{
    Q_D(CredentialHttp);
    d->passwordType = type;
}

QString CredentialHttp::passwordPreSalt() const
{
    Q_D(const CredentialHttp);
    return d->passwordPreSalt;
}

void CredentialHttp::setPasswordPreSalt(const QString &passwordPreSalt)
{
    Q_D(CredentialHttp);
    d->passwordPreSalt = passwordPreSalt;
}

QString CredentialHttp::passwordPostSalt() const
{
    Q_D(const CredentialHttp);
    return d->passwordPostSalt;
}

void CredentialHttp::setPasswordPostSalt(const QString &passwordPostSalt)
{
    Q_D(CredentialHttp);
    d->passwordPostSalt = passwordPostSalt;
}

QString CredentialHttp::usernameField() const
{
    Q_D(const CredentialHttp);
    return d->usernameField;
}

void CredentialHttp::setUsernameField(const QString &fieldName)
{
    Q_D(CredentialHttp);
    d->usernameField = fieldName;
}

void CredentialHttp::setRequireSsl(bool require)
{
    Q_D(CredentialHttp);
    d->requireSsl = require;
}

AuthenticationUser CredentialHttp::authenticate(Cutelyst::Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    Q_D(CredentialHttp);

    AuthenticationUser ret;
    if (d->requireSsl && !c->request()->secure()) {
        return d->authenticationFailed(c, realm, authinfo);
    }

    if (d->isAuthTypeDigest()) {
        ret = d->authenticateDigest(c, realm, authinfo);
        if (!ret.isNull()) {
            return ret;
        }
    }

    if (d->isAuthTypeBasic()) {
        ret = d->authenticateBasic(c, realm, authinfo);
        if (!ret.isNull()) {
            return ret;
        }
    }

    return d->authenticationFailed(c, realm, authinfo);
}

bool CredentialHttpPrivate::checkPassword(const AuthenticationUser &user, const ParamsMultiMap &authinfo)
{
    QString password = authinfo.value(passwordField);
    const QString storedPassword = user.value(passwordField);

    if (passwordType == CredentialHttp::None) {
        qCDebug(C_CREDENTIALHTTP) << "CredentialPassword is set to ignore password check";
        return true;
    } else if (passwordType == CredentialHttp::Clear) {
        return storedPassword == password;
    } else if (passwordType == CredentialHttp::Hashed) {
        if (!passwordPreSalt.isNull()) {
            password.prepend(password);
        }

        if (!passwordPostSalt.isNull()) {
            password.append(password);
        }

        return CredentialPassword::validatePassword(password.toUtf8(), storedPassword.toUtf8());
    } else if (passwordType == CredentialHttp::SelfCheck) {
        return user.checkPassword(password);
    }

    return false;
}

AuthenticationUser CredentialHttpPrivate::authenticateDigest(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    qCDebug(C_CREDENTIALHTTP) << "Checking http digest authentication.";

    return AuthenticationUser();
}

AuthenticationUser CredentialHttpPrivate::authenticateBasic(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    Q_UNUSED(authinfo)
    qCDebug(C_CREDENTIALHTTP) << "Checking http basic authentication.";

    QPair<QString, QString> userPass = c->req()->headers().authorizationBasicPair();
    if (userPass.first.isEmpty()) {
        return AuthenticationUser();
    }

    ParamsMultiMap auth;
    auth.insert(usernameField, userPass.first);
    AuthenticationUser user = realm->findUser(c, auth);
    if (!user.isNull()) {
        auth.insert(passwordField, userPass.second);
        if (checkPassword(user, auth)) {
            return user;
        }
        qCDebug(C_CREDENTIALHTTP) << "Password didn't match";
    } else {
        qCDebug(C_CREDENTIALHTTP) << "Unable to locate a user matching user info provided in realm";
    }
    return AuthenticationUser();
}

AuthenticationUser CredentialHttpPrivate::authenticationFailed(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    Response *res = c->response();
    res->setStatus(Response::Unauthorized); // 401
    res->setContentType(QStringLiteral("text/plain; charset=UTF-8"));

    if (authorizationRequiredMessage.isNull()) {
        res->body() = QByteArrayLiteral("Authorization required.");
    } else {
        res->body() = authorizationRequiredMessage.toUtf8();
    }

    // Create Digest response
    if (isAuthTypeDigest()) {
//        _create_digest_auth_response TODO
    }

    // Create Basic response
    if (isAuthTypeBasic()) {
        createBasicAuthResponse(c);
    }

    return AuthenticationUser();
}

bool CredentialHttpPrivate::isAuthTypeDigest() const
{
    return type == CredentialHttp::Digest || type == CredentialHttp::Any;

}

bool CredentialHttpPrivate::isAuthTypeBasic() const
{
    return type == CredentialHttp::Basic || type == CredentialHttp::Any;
}

void CredentialHttpPrivate::createBasicAuthResponse(Context *c)
{
    c->res()->headers().setWwwAuthenticate(joinAuthHeaderParts(QStringLiteral("Basic"),
                                                               buildAuthHeaderCommon()));
}

QStringList CredentialHttpPrivate::buildAuthHeaderCommon() const
{
    // TODO
    // return realm="realmname"
    // return domain="realmname"
    return QStringList();
}

QString CredentialHttpPrivate::joinAuthHeaderParts(const QString &type, const QStringList &parts) const
{
    if (parts.isEmpty()) {
        return type;
    } else {
        return type % QLatin1Char(' ') % parts.join(QStringLiteral(", "));
    }
}
