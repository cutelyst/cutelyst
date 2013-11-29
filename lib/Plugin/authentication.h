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

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "plugin.h"

#include <QObject>

class Cutelyst;
namespace CutelystPlugin {

class AuthenticationPrivate;
class Authentication : public Plugin
{
    Q_OBJECT
public:
    class User;
    class Realm;
    class Credential {
    public:
        virtual User authenticate(Cutelyst *c, Realm *realm, const User &authinfo);
    };

    class Store {
    public:
        /**
         * Reimplement this if your store supports
         * automatic user creation
         */
        virtual bool canAutoCreateUser() const;

        /**
         * Reimplement this if your store supports
         * automatic user creation
         */
        virtual bool autoCreateUser(Cutelyst *c, const CStringHash &userinfo) const;

        /**
         * Reimplement this if your store supports
         * automatic user update
         */
        virtual bool canAutoUpdateUser() const;

        /**
         * Reimplement this if your store supports
         * automatic user update
         */
        virtual bool autoUpdateUser(Cutelyst *c, const CStringHash &userinfo) const;

        User forSession(Cutelyst *c, const User &user);
    };

    class Realm
    {
    public:
        Realm(Store *store, Credential *credential);
        virtual bool findUser(Cutelyst *c, const CStringHash &userinfo);
        virtual User authenticate(Cutelyst *c, const User &authinfo);

    protected:
        User persistUser(Cutelyst *c, const Authentication::User &user);
        User restoreUser(Cutelyst *c, const User &frozenUser);
        User userIsRestorable(Cutelyst *c);

        Authentication *m_autehntication;

    private:
        friend class Authentication;

        Store *m_store;
        Credential *m_credential;
        QString m_name;
    };

    explicit Authentication(QObject *parent = 0);

    void addRealm(const QString &name, Authentication::Realm *realm, bool defaultRealm = true);

    void setUseSession(bool use);
    bool useSession() const;

    User authenticate(Cutelyst *c, const QString &username, const QString &password, const QString &realm = QString());
    User authenticate(Cutelyst *c, const User &userinfo, const QString &realm = QString());
    bool findUser(Cutelyst *c, const CStringHash &userinfo, const QString &realm = QString());
    User user(Cutelyst *c);
    bool userExists(Cutelyst *c) const;
    bool userInRealm(Cutelyst *c, const QString &realm) const;
    void logout(Cutelyst *c);

protected:
    void setAuthenticated(Cutelyst *c, const User &user, const QString &realmName);
    void persistUser(Cutelyst *c, const User &user, const QString &realmName);
    User restoreUser(Cutelyst *c, const User &frozenUser, const QString &realmName);
    Realm* findRealmForPersistedUser(Cutelyst *c);

    AuthenticationPrivate *d_ptr;

private:
    friend class Credential;
    Q_DECLARE_PRIVATE(Authentication)
};

class Nuts : public Authentication::Realm
{

};

}

#endif // AUTHENTICATION_H
