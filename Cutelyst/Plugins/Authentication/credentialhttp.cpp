/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "authenticationrealm.h"
#include "credentialhttp_p.h"
#include "credentialpassword.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QLoggingCategory>
#include <QUrl>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALHTTP, "cutelyst.plugin.credentialhttp", QtWarningMsg)

CredentialHttp::CredentialHttp(QObject *parent)
    : AuthenticationCredential(parent)
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

AuthenticationUser CredentialHttp::authenticate(Cutelyst::Context *c,
                                                AuthenticationRealm *realm,
                                                const ParamsMultiMap &authinfo)
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

bool CredentialHttpPrivate::checkPassword(const AuthenticationUser &user,
                                          const ParamsMultiMap &authinfo)
{
    const QString password = passwordPreSalt + authinfo.value(passwordField) + passwordPostSalt;
    const QString storedPassword = user.value(passwordField).toString();

    if (Q_LIKELY(passwordType == CredentialHttp::Hashed)) {
        return CredentialPassword::validatePassword(password.toUtf8(), storedPassword.toUtf8());
    } else if (passwordType == CredentialHttp::Clear) {
        return storedPassword == password;
    } else if (passwordType == CredentialHttp::None) {
        qCCritical(C_CREDENTIALHTTP) << "CredentialPassword is set to ignore password check";
        return true;
    }

    return false;
}

AuthenticationUser CredentialHttpPrivate::authenticateBasic(Context *c,
                                                            AuthenticationRealm *realm,
                                                            const ParamsMultiMap &authinfo)
{
    Q_UNUSED(authinfo)
    AuthenticationUser user;
    qCDebug(C_CREDENTIALHTTP) << "Checking http basic authentication.";

    const auto userPass = c->req()->headers().authorizationBasicObject();
    if (userPass.user.isEmpty()) {
        return user;
    }

    ParamsMultiMap auth;
    auth.insert(usernameField, userPass.user);
    AuthenticationUser _user = realm->findUser(c, auth);
    if (!_user.isNull()) {
        auth.insert(passwordField, userPass.password);
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

AuthenticationUser CredentialHttpPrivate::authenticationFailed(Context *c,
                                                               AuthenticationRealm *realm,
                                                               const ParamsMultiMap &authinfo)
{
    Q_UNUSED(authinfo);
    Response *res = c->response();
    res->setStatus(Response::Unauthorized); // 401
    res->setContentType("text/plain; charset=UTF-8"_qba);

    if (authorizationRequiredMessage.isEmpty()) {
        res->setBody("Authorization required."_qba);
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
    c->res()->headers().setWwwAuthenticate(
        joinAuthHeaderParts("Basic"_qba, buildAuthHeaderCommon(realm)));
}

QByteArrayList CredentialHttpPrivate::buildAuthHeaderCommon(AuthenticationRealm *realm) const
{
    QByteArrayList ret;
    // TODO
    // return realm="realmname"
    // return domain="realmname"
    if (!realm->name().isEmpty()) {
        ret.append("realm=\"" + realm->name().toLatin1() + '"');
    }
    return ret;
}

QByteArray CredentialHttpPrivate::joinAuthHeaderParts(const QByteArray &type,
                                                      const QByteArrayList &parts) const
{
    QByteArray ret = type;
    if (!parts.isEmpty()) {
        ret.append(' ' + parts.join(", "));
    }
    return ret;
}

#include "moc_credentialhttp.cpp"
