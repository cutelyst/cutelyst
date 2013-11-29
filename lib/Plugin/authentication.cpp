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
    Plugin(parent)
{
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

QString Authentication::authenticate(Cutelyst *c, const QString &username, const QString &password, const QString &realm)
{
    CStringHash userinfo;
    userinfo.insert(QLatin1String("username"), username);
    userinfo.insert(QLatin1String("password"), password);
    return authenticate(c, userinfo, realm);
}

QString Authentication::authenticate(Cutelyst *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->authenticate(c, userinfo);
    }

    qWarning() << Q_FUNC_INFO << "Could not find realm" << realm;
    return QString();
}

bool Authentication::findUser(Cutelyst *c, const CStringHash &userinfo, const QString &realm)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr = d->realm(realm);
    if (realmPtr) {
        return realmPtr->findUser(c, userinfo);
    }

    qWarning() << Q_FUNC_INFO << "Could not find realm" << realm;
    return false;
}

QString Authentication::user(Cutelyst *c)
{
    QVariant user = pluginProperty(c, "user");
    if (user.isNull()) {
        return restoreUser(c, QString(), QString());
    }
    return user.toString();
}

bool Authentication::userExists(Cutelyst *c) const
{
    return false;
}

bool Authentication::userInRealm(Cutelyst *c, const QString &realm) const
{
    return false;
}

void Authentication::logout(Cutelyst *c)
{

}

void Authentication::setAuthenticated(Cutelyst *c, const QString &user, const QString &realmName)
{
    Q_D(Authentication);

    setPluginProperty(c, "user", user);

    Authentication::Realm *realmPtr = d->realm(realmName);
    if (!realmPtr) {
        qWarning() << Q_FUNC_INFO << "Called with invalid realm" << realmName;
    }
    // TODO implement a user class
//    $user->auth_realm($realm->name);

    persistUser(c, user, realmName);
}

void Authentication::persistUser(Cutelyst *c, const QString &user, const QString &realmName)
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

QString Authentication::restoreUser(Cutelyst *c, const QString &frozenUser, const QString &realmName)
{
    Q_D(Authentication);

    Authentication::Realm *realmPtr;
    if (!realmName.isNull()) {
//        c = d->realm(realmName);
    } else {
        realmPtr = findRealmForPersistedUser(c);
    }

    QString user;
    if (realmPtr) {
        return realmPtr->restoreUser(c, frozenUser);
    }

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

bool Authentication::Realm::findUser(Cutelyst *c, const CStringHash &userinfo)
{
    bool ret = m_store;

    if (!ret) {
        if (m_store->canAutoCreateUser()) {
            ret = m_store->autoCreateUser(c, userinfo);
        }
    } else if (m_store->canAutoUpdateUser()) {
        ret = m_store->autoUpdateUser(c, userinfo);
    }

    return ret;
}

QString Authentication::Realm::authenticate(Cutelyst *c, const CStringHash &authinfo)
{
    QString user = m_credential->authenticate(c, this, authinfo);
    if (!user.isNull()) {
        c->plugin<Authentication*>()->setAuthenticated(c, user, m_name);
    }
    return user;
}

QString Authentication::Realm::persistUser(Cutelyst *c, const QString &user)
{
//    Session *session = c->plugin<Session*>();
//    if (session && session->isValid(c)) {
//        session->setValue(c, "Authentication::userRealm", m_name);
//    }

    m_store->forSession(c, user);
}

QString Authentication::Realm::restoreUser(Cutelyst *c, const QString &frozenUser)
{
    QString user = frozenUser;
    if (user.isNull()) {
        user = userIsRestorable(c);
    }

    // TODO restore a User object

    return user;
}

QString Authentication::Realm::userIsRestorable(Cutelyst *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        return session->value(c, "Authentication::user").toString();
    }
    return QString();
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

bool Authentication::Store::autoCreateUser(Cutelyst *c, const CStringHash &userinfo) const
{
    return false;
}

bool Authentication::Store::canAutoUpdateUser() const
{
    return false;
}

bool Authentication::Store::autoUpdateUser(Cutelyst *c, const CStringHash &userinfo) const
{
    return false;
}

QString Authentication::Store::forSession(Cutelyst *c, const QString &user)
{
    return QString();
}


QString Authentication::Credential::authenticate(Cutelyst *c, Realm *realm, const CStringHash &authinfo)
{
    return QString();
}
