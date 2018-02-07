/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "authentication_p.h"

#include "authenticationstore.h"
#include "authenticationrealm.h"

#include "context.h"
#include "application.h"

#include <Cutelyst/Plugins/Session/session.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_UTILS_AUTH, "cutelyst.utils.auth")
Q_LOGGING_CATEGORY(C_AUTHENTICATION, "cutelyst.plugin.authentication")

using namespace Cutelyst;

char *Authentication::defaultRealm = const_cast<char *>("cutelyst_authentication_default_realm");
char *AuthenticationRealm::defaultRealm = const_cast<char *>("cutelyst_authentication_default_realm");

static thread_local Authentication *auth = nullptr;

#define AUTHENTICATION_USER "__authentication_user"
#define AUTHENTICATION_USER_REALM "__authentication_user_realm"

Authentication::Authentication(Application *parent) : Plugin(parent)
  , d_ptr(new AuthenticationPrivate)
{
    qRegisterMetaType<AuthenticationUser>();
    qRegisterMetaTypeStreamOperators<AuthenticationUser>();
}

Authentication::~Authentication()
{
    delete d_ptr;
}

void Authentication::addRealm(Cutelyst::AuthenticationRealm *realm)
{
    Q_D(Authentication);
    realm->setParent(this);
    d->realms.insert(realm->objectName(), realm);
    d->realmsOrder.append(realm->objectName());
}

void Cutelyst::Authentication::addRealm(Cutelyst::AuthenticationStore *store, Cutelyst::AuthenticationCredential *credential, const QString &name)
{
    addRealm(new AuthenticationRealm(store, credential, name, this));
}

AuthenticationRealm *Authentication::realm(const QString &name) const
{
    Q_D(const Authentication);
    return d->realms.value(name);
}

bool Authentication::authenticate(Cutelyst::Context *c, const ParamsMultiMap &userinfo, const QString &realm)
{
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return false;
    }

    AuthenticationRealm *realmPtr = auth->d_ptr->realm(realm);
    if (realmPtr) {
        const AuthenticationUser user = realmPtr->authenticate(c, userinfo);
        if (!user.isNull()) {
            AuthenticationPrivate::setAuthenticated(c, user, realm, auth->d_ptr->realm(realm));
        }

        return !user.isNull();
    }

    qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
    return false;
}

AuthenticationUser Authentication::findUser(Cutelyst::Context *c, const ParamsMultiMap &userinfo, const QString &realm)
{
    AuthenticationUser ret;
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return ret;
    }

    AuthenticationRealm *realmPtr = auth->d_ptr->realm(realm);
    if (!realmPtr) {
        qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
        return ret;
    }

    ret = realmPtr->findUser(c, userinfo);
    return ret;
}

Cutelyst::AuthenticationUser Authentication::user(Cutelyst::Context *c)
{
    AuthenticationUser ret;
    QVariant user = c->property(AUTHENTICATION_USER);
    if (user.isNull()) {        
        ret = AuthenticationPrivate::restoreUser(c, QVariant(), QString());
    } else {
        ret = user.value<AuthenticationUser>();
    }
    return ret;
}

bool Authentication::userExists(Cutelyst::Context *c)
{
    if (!c->property(AUTHENTICATION_USER).isNull()) {
        return true;
    } else {
        if (auth) {
            if (AuthenticationPrivate::findRealmForPersistedUser(c, auth->d_ptr->realms, auth->d_ptr->realmsOrder)) {
                return true;
            }
        } else {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
        }
        return false;
    }
}

bool Authentication::userInRealm(Cutelyst::Context *c, const QString &realmName)
{
    QVariant user = c->property(AUTHENTICATION_USER);
    if (!user.isNull()) {
        return user.value<AuthenticationUser>().authRealm() == realmName;
    } else {
        if (!auth) {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
            return false;
        }

        AuthenticationRealm *realm = AuthenticationPrivate::findRealmForPersistedUser(c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realm) {
            return realm->name() == realmName;
        } else {
            return false;
        }
    }
}

void Authentication::logout(Context *c)
{
    AuthenticationPrivate::setUser(c, AuthenticationUser());

    if (auth) {
        AuthenticationRealm *realm = AuthenticationPrivate::findRealmForPersistedUser(c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realm) {
            realm->removePersistedUser(c);
        }
    } else {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
    }
}

bool Authentication::setup(Application *app)
{
    connect(app, &Application::postForked, this, &AuthenticationPrivate::_q_postFork);
    return true;
}

AuthenticationRealm *AuthenticationPrivate::realm(const QString &realmName) const
{
    return realms.value(realmName.isNull() ? defaultRealm : realmName);
}

AuthenticationUser AuthenticationPrivate::restoreUser(Context *c, const QVariant &frozenUser, const QString &realmName)
{
    AuthenticationUser ret;
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return ret;
    }

    AuthenticationRealm *realmPtr = auth->d_ptr->realm(realmName);
    if (!realmPtr) {
        realmPtr = AuthenticationPrivate::findRealmForPersistedUser(c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
    }

    if (!realmPtr) {
        return ret;
    }

    ret = realmPtr->restoreUser(c, frozenUser);

    AuthenticationPrivate::setUser(c, ret);
    // TODO
    // $user->auth_realm($realm->name) if $user;

    return ret;
}

AuthenticationRealm *AuthenticationPrivate::findRealmForPersistedUser(Context *c, const QMap<QString, AuthenticationRealm *> &realms, const QStringList &realmsOrder)
{
    AuthenticationRealm *realm;

    const QVariant realmVariant = Session::value(c, QStringLiteral(AUTHENTICATION_USER_REALM));
    if (!realmVariant.isNull()) {
        realm = realms.value(realmVariant.toString());
        if (realm && !realm->userIsRestorable(c).isNull()) {
            return realm;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        for (const QString &realmName : realmsOrder) {
            AuthenticationRealm *realm = realms.value(realmName);
            if (realm && !realm->userIsRestorable(c).isNull()) {
                return realm;
            }
        }
    }

    return nullptr;
}

void AuthenticationPrivate::setAuthenticated(Context *c, const AuthenticationUser &user, const QString &realmName, AuthenticationRealm *realm)
{
    AuthenticationPrivate::setUser(c, user);

    if (!realm) {
        qCWarning(C_AUTHENTICATION) << "Called with invalid realm" << realmName;
    }
    // TODO implement a user class
//    $user->auth_realm($realm->name);

    AuthenticationPrivate::persistUser(c, user, realmName, realm);
}

void AuthenticationPrivate::setUser(Context *c, const AuthenticationUser &user, const QString &realmName)
{
    if (user.isNull()) {
        c->setProperty(AUTHENTICATION_USER, QVariant());
        c->setProperty(AUTHENTICATION_USER_REALM, QVariant());
    } else {
        c->setProperty(AUTHENTICATION_USER, QVariant::fromValue(user));
        c->setProperty(AUTHENTICATION_USER_REALM, realmName);
    }
}

void AuthenticationPrivate::persistUser(Context *c, const AuthenticationUser &user, const QString &realmName, AuthenticationRealm *realm)
{
    if (Authentication::userExists(c)) {
        if (Session::isValid(c)) {
            Session::setValue(c, QStringLiteral(AUTHENTICATION_USER_REALM), realmName);
        }

        if (realm) {
            realm->persistUser(c, user);
        }
    }
}

void AuthenticationPrivate::_q_postFork(Application *app)
{
    auth = app->plugin<Authentication *>();
}

Cutelyst::AuthenticationCredential::AuthenticationCredential(QObject *parent) : QObject(parent)
{

}

Cutelyst::AuthenticationCredential::~AuthenticationCredential()
{

}

#include "moc_authentication.cpp"
