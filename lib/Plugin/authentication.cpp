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

QString Authentication::user(Cutelyst *c) const
{

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


QString Authentication::Credential::authenticate(Cutelyst *c, Realm *realm, const CStringHash &authinfo)
{
    return QString();
}
