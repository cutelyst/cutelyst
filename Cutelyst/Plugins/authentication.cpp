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
#include "context.h"
#include "session.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_UTILS_AUTH, "cutelyst.utils.auth")

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_AUTHENTICATION, "cutelyst.plugin.authentication")

char *Authentication::defaultRealm = const_cast<char *>("cutelyst_authentication_default_realm");

Authentication::Authentication(Application *parent) : Plugin(parent)
  , d_ptr(new AuthenticationPrivate)
{
    qRegisterMetaType<User>();
    qRegisterMetaTypeStreamOperators<User>();
}

Authentication::~Authentication()
{
    delete d_ptr;
}

bool Authentication::setup(Cutelyst::Application *app)
{
    Q_D(Authentication);
    return true;
}

void Authentication::addRealm(Authentication::Realm *realm, const QString &name)
{
    Q_D(Authentication);
    d->realms.insert(name, realm);
    d->realmsOrder.append(name);
}

Authentication::Realm *Authentication::realm(const QString &name) const
{
    Q_D(const Authentication);
    return d->realms.value(name);
}

Authentication::User Authentication::authenticate(Cutelyst::Context *c, const QString &username, const QString &password, const QString &realm)
{
    return authenticate(c, {
                            {QStringLiteral("username"), username},
                            {QStringLiteral("password"), password}
                        },
                        realm);
}

Authentication::User Authentication::authenticate(Cutelyst::Context *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        User user = realmPtr->authenticate(c, userinfo);
        if (!user.isNull()) {
            setAuthenticated(c, user, realm);
        }

        return user;
    }

    qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
    return User();
}

Authentication::User Authentication::findUser(Cutelyst::Context *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->findUser(c, userinfo);
    }

    qCWarning(C_AUTHENTICATION)   << "Could not find realm" << realm;
    return User();
}

Cutelyst::Authentication::User Cutelyst::Authentication::user(Cutelyst::Context *c)
{
    Q_D(Authentication);
    QVariant user = pluginProperty(c, QStringLiteral("user"));
    if (user.isNull()) {        
        return restoreUser(c, QVariant(), QString());
    }
    return user.value<User>();
}

void Authentication::setUser(Context *c, const Authentication::User &user)
{
    Q_D(Authentication);
    if (user.isNull()) {
        setPluginProperty(c, QStringLiteral("user"), QVariant());
    } else {
        setPluginProperty(c, QStringLiteral("user"), QVariant::fromValue(user));
    }
}

bool Authentication::userExists(Cutelyst::Context *c)
{
    return !user(c).isNull();
}

bool Authentication::userInRealm(Cutelyst::Context *c, const QString &realm)
{
    QVariant user = pluginProperty(c, QStringLiteral("user"));
    if (user.isNull()) {
        return !restoreUser(c, QVariant(), realm).isNull();
    }
    return false;
}

void Authentication::logout(Context *c)
{
    setUser(c, User());

    Authentication::Realm *realm = findRealmForPersistedUser(c);
    if (realm) {
        realm->removePersistedUser(c);
    }
}

void Authentication::setAuthenticated(Cutelyst::Context *c, const User &user, const QString &realmName)
{
    Q_D(Authentication);

    setUser(c, user);

    Authentication::Realm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        qCWarning(C_AUTHENTICATION) << "Called with invalid realm" << realmName;
    }
    // TODO implement a user class
//    $user->auth_realm($realm->name);

    persistUser(c, user, realmName);
}

void Authentication::persistUser(Context *c, const User &user, const QString &realmName)
{
    Q_D(Authentication);

    if (userExists(c)) {
        Session *session = c->plugin<Session*>();
        if (session && session->isValid(c)) {
            session->setValue(c, QStringLiteral("Authentication::userRealm"), realmName);
        }

        Authentication::Realm *realmPtr = d->realm(realmName);
        if (realmPtr) {
            realmPtr->persistUser(c, user);
        }
    }
}

Authentication::User Authentication::restoreUser(Cutelyst::Context *c, const QVariant &frozenUser, const QString &realmName)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        realmPtr = findRealmForPersistedUser(c);
    }

    if (!realmPtr) {
        return User();
    }

    User user = realmPtr->restoreUser(c, frozenUser);

    setUser(c, user);
    // TODO
    // $user->auth_realm($realm->name) if $user;

    return user;
}

Authentication::Realm *Authentication::findRealmForPersistedUser(Cutelyst::Context *c)
{
    Q_D(Authentication);

    Authentication::Realm *realm;

    Session *session = c->plugin<Session*>();
    if (session &&
            session->isValid(c) &&
            !session->value(c, QStringLiteral("Authentication::userRealm")).isNull()) {
        const QString &realmName = session->value(c, QStringLiteral("Authentication::userRealm")).toString();
        realm = d->realms.value(realmName);
        if (realm && !realm->userIsRestorable(c).isNull()) {
            return realm;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        Q_FOREACH (const QString &realmName, d->realmsOrder) {
            Authentication::Realm *realm = d->realms.value(realmName);
            if (realm && !realm->userIsRestorable(c).isNull()) {
                return realm;
            }
        }
    }

    return 0;
}

Authentication::Realm::Realm(AuthenticationStore *store, Authentication::Credential *credential) :
    m_store(store),
    m_credential(credential)
{

}

AuthenticationStore *Authentication::Realm::store() const
{
    return m_store;
}

Authentication::Credential *Authentication::Realm::credential() const
{
    return m_credential;
}

Authentication::User Authentication::Realm::findUser(Context *ctx, const CStringHash &userinfo)
{
    User ret = m_store->findUser(ctx, userinfo);

    if (ret.isNull()) {
        if (m_store->canAutoCreateUser()) {
            ret = m_store->autoCreateUser(ctx, userinfo);
        }
    } else if (m_store->canAutoUpdateUser()) {
        ret = m_store->autoUpdateUser(ctx, userinfo);
    }

    return ret;
}

Authentication::User Authentication::Realm::authenticate(Context *ctx, const CStringHash &authinfo)
{
    return m_credential->authenticate(ctx, this, authinfo);
}

void Authentication::Realm::removePersistedUser(Context *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->deleteValue(c, QStringLiteral("Authentication::user"));
        session->deleteValue(c, QStringLiteral("Authentication::userRealm"));
    }
}

Authentication::User Authentication::Realm::persistUser(Context *c, const Authentication::User &user)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->setValue(c,
                          QStringLiteral("Authentication::user"),
                          m_store->forSession(c, user));
    }

    return user;
}

Authentication::User Authentication::Realm::restoreUser(Context *ctx, const QVariant &frozenUser)
{
    QVariant _frozenUser = frozenUser;
    if (_frozenUser.isNull()) {
        _frozenUser = userIsRestorable(ctx);
    }

    if (_frozenUser.isNull()) {
        return User();
    }

    User user = m_store->fromSession(ctx, _frozenUser);

    if (!user.isNull()) {
        ctx->plugin<Authentication*>()->setUser(ctx, user);

        // Sets the realm the user originated in
        user.setAuthRealm(this);
    } else {
        qCWarning(C_AUTHENTICATION) << "Store claimed to have a restorable user, but restoration failed. Did you change the user's id_field?";
    }

    return user;
}

QVariant Authentication::Realm::userIsRestorable(Context *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        return session->value(c, QStringLiteral("Authentication::user"));
    }
    return QVariant();
}

Authentication::Realm *AuthenticationPrivate::realm(const QString &realmName) const
{
    QString name = realmName;
    if (name.isNull()) {
        name = defaultRealm;
    }
    return realms.value(name);
}

Authentication::User::User()
{

}

Authentication::User::User(const QString &id) :
    m_id(id)
{

}

Authentication::User::~User()
{
}

QString Authentication::User::id() const
{
    return m_id;
}

void Authentication::User::setId(const QString &id)
{
    m_id = id;
}

bool Authentication::User::isNull() const
{
    return m_id.isNull();
}

Authentication::Realm *Authentication::User::authRealm()
{
    return m_realm;
}

void Authentication::User::setAuthRealm(Authentication::Realm *authRealm)
{
    m_realm = authRealm;
}

bool Authentication::User::checkPassword(const QString &password) const
{
    Q_UNUSED(password)
    return false;
}

QDataStream &operator<<(QDataStream &out, const Authentication::User &user)
{
    out << user.id() << static_cast<CStringHash>(user);
    return out;
}

QDataStream &operator>>(QDataStream &in, Authentication::User &user)
{
    QString id;
    CStringHash hash;
    in >> id >> hash;
    user.setId(id);
    user.swap(hash);
    return in;
}
