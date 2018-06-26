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
#include "credentialhttp_p.h"
#include "credentialpassword.h"

#include "authenticationrealm.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QUrl>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALHTTP, "cutelyst.plugin.credentialhttp", QtWarningMsg)

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
        ret = d->authenticationFailed(c, realm, authinfo);
        return ret;
    }

    if (d->isAuthTypeBasic()) {
        ret = d->authenticateBasic(c, realm, authinfo);
        if (!ret.isNull()) {
            return ret;
        }
    }

    ret = d->authenticationFailed(c, realm, authinfo);
    return ret;
}

bool CredentialHttpPrivate::checkPassword(const AuthenticationUser &user, const ParamsMultiMap &authinfo)
{
    QString password = authinfo.value(passwordField);
    const QString storedPassword = user.value(passwordField).toString();

    if (Q_LIKELY(passwordType == CredentialHttp::Hashed)) {
        if (!passwordPreSalt.isEmpty()) {
            password.prepend(password);
        }

        if (!passwordPostSalt.isEmpty()) {
            password.append(password);
        }

        return CredentialPassword::validatePassword(password.toUtf8(), storedPassword.toUtf8());
    } else if (passwordType == CredentialHttp::Clear) {
        return storedPassword == password;
    } else if (passwordType == CredentialHttp::None) {
        qCCritical(C_CREDENTIALHTTP) << "CredentialPassword is set to ignore password check";
        return true;
    }

    return false;
}

AuthenticationUser CredentialHttpPrivate::authenticateBasic(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    Q_UNUSED(authinfo)
    AuthenticationUser user;
    qCDebug(C_CREDENTIALHTTP) << "Checking http basic authentication.";

    const std::pair<QString, QString> userPass = c->req()->headers().authorizationBasicPair();
    if (userPass.first.isEmpty()) {
        return user;
    }

    ParamsMultiMap auth;
    auth.insert(usernameField, userPass.first);
    AuthenticationUser _user = realm->findUser(c, auth);
    if (!_user.isNull()) {
        auth.insert(passwordField, userPass.second);
        if (checkPassword(_user, auth)) {
            user = _user;
        } else {
            qCDebug(C_CREDENTIALHTTP) << "Password didn't match";
        }
    } else {
        qCDebug(C_CREDENTIALHTTP) << "Unable to locate a user matching user info provided in realm";
    }
    return user;
}

AuthenticationUser CredentialHttpPrivate::authenticationFailed(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo)
{
    Response *res = c->response();
    res->setStatus(Response::Unauthorized); // 401
    res->setContentType(QStringLiteral("text/plain; charset=UTF-8"));

    if (authorizationRequiredMessage.isEmpty()) {
        res->setBody(QStringLiteral("Authorization required."));
    } else {
        res->setBody(authorizationRequiredMessage);
    }

    // Create Basic response
    if (isAuthTypeBasic()) {
        createBasicAuthResponse(c, realm);
    }

    return AuthenticationUser();
}

bool CredentialHttpPrivate::isAuthTypeBasic() const
{
    return type == CredentialHttp::Basic || type == CredentialHttp::Any;
}

void CredentialHttpPrivate::createBasicAuthResponse(Context *c, AuthenticationRealm *realm)
{
    c->res()->headers().setWwwAuthenticate(joinAuthHeaderParts(QStringLiteral("Basic"),
                                                               buildAuthHeaderCommon(realm)));
}

QStringList CredentialHttpPrivate::buildAuthHeaderCommon(AuthenticationRealm *realm) const
{
    QStringList ret;
    // TODO
    // return realm="realmname"
    // return domain="realmname"
    if (!realm->name().isEmpty()) {
        ret.append(QLatin1String("realm=\"") + realm->name() + QLatin1Char('"'));
    }
    return ret;
}

QString CredentialHttpPrivate::joinAuthHeaderParts(const QString &type, const QStringList &parts) const
{
    QString ret = type;
    if (!parts.isEmpty()) {
        ret.append(QLatin1Char(' ') + parts.join(QStringLiteral(", ")));
    }
    return ret;
}

#include "moc_credentialhttp.cpp"
