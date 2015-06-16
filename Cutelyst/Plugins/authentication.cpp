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

#include "authentication_p.h"

#include "authenticationstore.h"
#include "authenticationrealm.h"
#include "context.h"
#include "session.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_UTILS_AUTH, "cutelyst.utils.auth")
Q_LOGGING_CATEGORY(C_AUTHENTICATION, "cutelyst.plugin.authentication")

using namespace Cutelyst;

char *Authentication::defaultRealm = const_cast<char *>("cutelyst_authentication_default_realm");

#define AUTHENTICATION_USER "__authentication_user"
#define SESSION_USER_REALM "__authentication_user_realm"
#define SESSION_AUTHENTICATION_USER_REALM "__authentication_user_realm" // in realm.cpp

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

void Authentication::addRealm(Cutelyst::AuthenticationRealm *realm, const QString &name)
{
    Q_D(Authentication);
    realm->setObjectName(name);
    d->realms.insert(name, realm);
    d->realmsOrder.append(name);
}

void Cutelyst::Authentication::addRealm(Cutelyst::AuthenticationStore *store, Cutelyst::AuthenticationCredential *credential, const QString &name)
{
    addRealm(new AuthenticationRealm(store, credential, this), name);
}

AuthenticationRealm *Authentication::realm(const QString &name) const
{
    Q_D(const Authentication);
    return d->realms.value(name);
}

bool Authentication::authenticate(Cutelyst::Context *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    AuthenticationRealm *realmPtr = d->realm(realm);
    if (realmPtr) {
        const AuthenticationUser &user = realmPtr->authenticate(c, userinfo);
        if (!user.isNull()) {
            setAuthenticated(c, user, realm);
        }

        return !user.isNull();
    }

    qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
    return false;
}

AuthenticationUser Authentication::findUser(Cutelyst::Context *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    AuthenticationRealm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->findUser(c, userinfo);
    }

    qCWarning(C_AUTHENTICATION)   << "Could not find realm" << realm;
    return AuthenticationUser();
}

Cutelyst::AuthenticationUser Cutelyst::Authentication::user(Cutelyst::Context *c)
{
    QVariant user = c->property(AUTHENTICATION_USER);
    if (user.isNull()) {        
        return restoreUser(c, QVariant(), QString());
    }
    return user.value<AuthenticationUser>();
}

void Authentication::setUser(Context *c, const AuthenticationUser &user)
{
    if (user.isNull()) {
        c->setProperty(AUTHENTICATION_USER, QVariant());
    } else {
        c->setProperty(AUTHENTICATION_USER, QVariant::fromValue(user));
    }
}

bool Authentication::userExists(Cutelyst::Context *c)
{
    if (!c->property(AUTHENTICATION_USER).isNull()) {
        return true;
    } else {
        Authentication *auth = c->plugin<Authentication*>();
        if (auth) {
            if (auth->findRealmForPersistedUser(c)) {
                return true;
            }
        } else {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
        }
        return false;
    }
}

bool Authentication::userInRealm(Cutelyst::Context *c, const QString &realm)
{
    QVariant user = c->property(AUTHENTICATION_USER);
    if (user.isNull()) {
        return !restoreUser(c, QVariant(), realm).isNull();
    }
    return false;
}

void Authentication::logout(Context *c)
{
    setUser(c, AuthenticationUser());

    Authentication *auth = c->plugin<Authentication*>();
    if (auth) {
        AuthenticationRealm *realm = auth->findRealmForPersistedUser(c);
        if (realm) {
            realm->removePersistedUser(c);
        }
    } else {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
    }
}

void Authentication::setAuthenticated(Cutelyst::Context *c, const AuthenticationUser &user, const QString &realmName)
{
    Q_D(Authentication);

    setUser(c, user);

    AuthenticationRealm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        qCWarning(C_AUTHENTICATION) << "Called with invalid realm" << realmName;
    }
    // TODO implement a user class
//    $user->auth_realm($realm->name);

    persistUser(c, user, realmName);
}

void Authentication::persistUser(Context *c, const AuthenticationUser &user, const QString &realmName)
{
    Q_D(Authentication);

    if (userExists(c)) {
        if (Session::isValid(c)) {
            Session::setValue(c, QStringLiteral(SESSION_AUTHENTICATION_USER_REALM), realmName);
        }

        AuthenticationRealm *realmPtr = d->realm(realmName);
        if (realmPtr) {
            realmPtr->persistUser(c, user);
        }
    }
}

AuthenticationUser Authentication::restoreUser(Cutelyst::Context *c, const QVariant &frozenUser, const QString &realmName)
{
    Q_D(Authentication);

    AuthenticationRealm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        realmPtr = findRealmForPersistedUser(c);
    }

    if (!realmPtr) {
        return AuthenticationUser();
    }

    AuthenticationUser user = realmPtr->restoreUser(c, frozenUser);

    setUser(c, user);
    // TODO
    // $user->auth_realm($realm->name) if $user;

    return user;
}

AuthenticationRealm *Authentication::findRealmForPersistedUser(Cutelyst::Context *c)
{
    Q_D(Authentication);

    AuthenticationRealm *realm;

    const QVariant &realmVariant = Session::value(c, QStringLiteral(SESSION_USER_REALM));
    if (!realmVariant.isNull()) {
        realm = d->realms.value(realmVariant.toString());
        if (realm && !realm->userIsRestorable(c).isNull()) {
            return realm;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        Q_FOREACH (const QString &realmName, d->realmsOrder) {
            AuthenticationRealm *realm = d->realms.value(realmName);
            if (realm && !realm->userIsRestorable(c).isNull()) {
                return realm;
            }
        }
    }

    return 0;
}

AuthenticationRealm *AuthenticationPrivate::realm(const QString &realmName) const
{
    QString name = realmName;
    if (name.isNull()) {
        name = defaultRealm;
    }
    return realms.value(name);
}



Cutelyst::AuthenticationCredential::AuthenticationCredential(QObject *parent) : QObject(parent)
{

}

Cutelyst::AuthenticationCredential::~AuthenticationCredential()
{

}
