/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "cutelyst.h"
#include "session.h"

#include <QDebug>

using namespace CutelystPlugin;

Authentication::Authentication(QObject *parent) :
    Plugin(parent),
    d_ptr(new AuthenticationPrivate)
{
    qRegisterMetaType<User>();
    qRegisterMetaTypeStreamOperators<User>();
}

Authentication::~Authentication()
{
    delete d_ptr;
}

void Authentication::addRealm(Authentication::Realm *realm)
{
    addRealm(QLatin1String("default"), realm);
}

void Authentication::addRealm(const QString &name, Authentication::Realm *realm, bool defaultRealm)
{
    Q_D(Authentication);
    if (defaultRealm) {
        d->defaultRealm = name;
    }
    d->realms.insert(name, realm);
    d->realmsOrder.append(name);
    realm->m_autehntication = this;
    realm->m_name = name;
}

void Authentication::setUseSession(bool use)
{

}

bool Authentication::useSession() const
{

}

Authentication::User Authentication::authenticate(Cutelyst *c, const QString &username, const QString &password, const QString &realm)
{
    CStringHash userinfo;
    userinfo.insert(QLatin1String("username"), username);
    userinfo.insert(QLatin1String("password"), password);
    return authenticate(c, userinfo, realm);
}

Authentication::User Authentication::authenticate(Cutelyst *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->authenticate(c, userinfo);
    }

    qWarning() << "Could not find realm" << realm;
    return User();
}

Authentication::User Authentication::findUser(Cutelyst *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->findUser(c, userinfo);
    }

    qWarning()   << "Could not find realm" << realm;
    return User();
}

Authentication::User Authentication::user(Cutelyst *c)
{
    QVariant user = pluginProperty(c, "user");
    if (user.isNull()) {
        return restoreUser(c, QVariant(), QString());
    }
    return user.value<User>();
}

void Authentication::setUser(Cutelyst *c, const Authentication::User &user)
{
    if (user.isNull()) {
        setPluginProperty(c, "user", QVariant());
    } else {
        setPluginProperty(c, "user", qVariantFromValue(user));
    }
}

bool Authentication::userExists(Cutelyst *c)
{
    return !user(c).isNull();
}

bool Authentication::userInRealm(Cutelyst *c, const QString &realm)
{
    QVariant user = pluginProperty(c, "user");
    if (user.isNull()) {
        return !restoreUser(c, QVariant(), realm).isNull();
    }
    return false;
}

void Authentication::logout(Cutelyst *c)
{
    setUser(c, User());

    Authentication::Realm *realm = findRealmForPersistedUser(c);
    if (realm) {
        realm->removePersistedUser(c);
    }
}

void Authentication::setAuthenticated(Cutelyst *c, const User &user, const QString &realmName)
{
    Q_D(Authentication);

    setUser(c, user);

    Authentication::Realm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        qWarning() << Q_FUNC_INFO << "Called with invalid realm" << realmName;
    }
    // TODO implement a user class
//    $user->auth_realm($realm->name);

    persistUser(c, user, realmName);
}

void Authentication::persistUser(Cutelyst *c, const User &user, const QString &realmName)
{
    Q_D(Authentication);

    if (userExists(c)) {
        Session *session = c->plugin<Session*>();
        if (session && session->isValid(c)) {
            session->setValue(c, "Authentication::userRealm", realmName);
        }

        Authentication::Realm *realmPtr = d->realm(realmName);
        if (realmPtr) {
            realmPtr->persistUser(c, user);
        }
    }
}

Authentication::User Authentication::restoreUser(Cutelyst *c, const QVariant &frozenUser, const QString &realmName)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realms.value(realmName);
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

Authentication::Realm *Authentication::findRealmForPersistedUser(Cutelyst *c)
{
    Q_D(Authentication);

    Authentication::Realm *realm;

    Session *session = c->plugin<Session*>();
    if (session &&
            session->isValid(c) &&
            !session->value(c, "Authentication::userRealm").isNull()) {
        QString realmName = session->value(c, "Authentication::userRealm").toString();
        realm = d->realms.value(realmName);
        if (realm && !realm->userIsRestorable(c).isNull()) {
            return realm;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        foreach (const QString &realmName, d->realmsOrder) {
            Authentication::Realm *realm = d->realms.value(realmName);
            if (realm && !realm->userIsRestorable(c).isNull()) {
                return realm;
            }
        }
    }

    return 0;
}

Authentication::Realm::Realm(Authentication::Store *store, Authentication::Credential *credential) :
    m_store(store),
    m_credential(credential)
{

}

Authentication::User Authentication::Realm::findUser(Cutelyst *c, const CStringHash &userinfo)
{
    User ret = m_store->findUser(c, userinfo);

    if (ret.isNull()) {
        if (m_store->canAutoCreateUser()) {
            ret = m_store->autoCreateUser(c, userinfo);
        }
    } else if (m_store->canAutoUpdateUser()) {
        ret = m_store->autoUpdateUser(c, userinfo);
    }

    return ret;
}

Authentication::User Authentication::Realm::authenticate(Cutelyst *c, const CStringHash &authinfo)
{
    User user = m_credential->authenticate(c, this, authinfo);
    if (!user.isNull()) {
        c->plugin<Authentication*>()->setAuthenticated(c, user, m_name);
    }

    return user;
}

void Authentication::Realm::removePersistedUser(Cutelyst *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->deleteValue(c, "Authentication::user");
        session->deleteValue(c, "Authentication::userRealm");
    }
}

Authentication::User Authentication::Realm::persistUser(Cutelyst *c, const Authentication::User &user)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->setValue(c, "Authentication::user",
                          m_store->forSession(c, user));
    }

    return user;
}

Authentication::User Authentication::Realm::restoreUser(Cutelyst *c, const QVariant &frozenUser)
{
    QVariant _frozenUser = frozenUser;
    if (_frozenUser.isNull()) {
        _frozenUser = userIsRestorable(c);
    }

    if (_frozenUser.isNull()) {
        return User();
    }

    User user = m_store->fromSession(c, _frozenUser);

    if (!user.isNull()) {
        c->plugin<Authentication*>()->setUser(c, user);

        // Sets the realm the user originated in
        user.setAuthRealm(this);
    } else {
        qWarning("Store claimed to have a restorable user, but restoration failed. Did you change the user's id_field?");
    }

    return user;
}

QVariant Authentication::Realm::userIsRestorable(Cutelyst *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        return session->value(c, "Authentication::user");
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


bool Authentication::Store::canAutoCreateUser() const
{
    return false;
}

Authentication::User Authentication::Store::autoCreateUser(Cutelyst *c, const CStringHash &userinfo) const
{
    return User();
}

bool Authentication::Store::canAutoUpdateUser() const
{
    return false;
}

Authentication::User Authentication::Store::autoUpdateUser(Cutelyst *c, const CStringHash &userinfo) const
{
    return User();
}

Authentication::User::User()
{

}

Authentication::User::User(const QString &id) :
    m_id(id)
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

QVariant CutelystPlugin::Authentication::Store::forSession(Cutelyst *c, const CutelystPlugin::Authentication::User &user)
{
    Q_UNUSED(c)
    return qVariantFromValue(user);
}

Authentication::User Authentication::Store::fromSession(Cutelyst *c, const QVariant &frozenUser)
{
    Q_UNUSED(c)
    return frozenUser.value<User>();
}
