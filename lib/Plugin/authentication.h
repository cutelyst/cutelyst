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
    class Realm;
    class User : public CStringHash
    {
    public:
        User();
        User(const QString &id);

        /**
         * A unique ID by which a user can be retrieved from the store.
         */
        QString id() const;
        void setId(const QString &id);
        bool isNull() const;

        Realm *authRealm();
        void setAuthRealm(Realm *authRealm);

    private:
        QString m_id;
        Realm *m_realm;
    };

    class Credential {
    public:
        virtual User authenticate(Context *ctx, Realm *realm, const CStringHash &authinfo) = 0;
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
        virtual User autoCreateUser(Context *ctx, const CStringHash &userinfo) const;

        /**
         * Reimplement this if your store supports
         * automatic user update
         */
        virtual bool canAutoUpdateUser() const;

        /**
         * Reimplement this if your store supports
         * automatic user update
         */
        virtual User autoUpdateUser(Context *ctx, const CStringHash &userinfo) const;

        /**
         * Retrieve the user that matches the user info
         */
        virtual User findUser(Context *ctx, const CStringHash &userinfo) = 0;

        /**
         * Reimplement this so that you return a
         * serializable value that can be used to
         * identify the user.
         * The default implementation just returns
         * the user.
         */
        virtual QVariant forSession(Context *ctx, const Authentication::User &user);

        /**
         * Reimplement this so that you return a
         * User that was stored in the session.
         *
         * The default implementation just returns
         * the user.
         */
        virtual User fromSession(Context *ctx, const QVariant &frozenUser);
    };

    class Realm
    {
    public:
        Realm(Store *store, Credential *credential);
        virtual User findUser(Context *ctx, const CStringHash &userinfo);
        virtual User authenticate(Context *ctx, const CStringHash &authinfo);

    protected:
        void removePersistedUser(Context *ctx);
        User persistUser(Context *ctx, const Authentication::User &user);
        User restoreUser(Context *ctx, const QVariant &frozenUser);
        QVariant userIsRestorable(Context *ctx);

        Authentication *m_autehntication;

    private:
        friend class Authentication;

        Store *m_store;
        Credential *m_credential;
        QString m_name;
    };

    explicit Authentication(QObject *parent = 0);
    ~Authentication();

    void addRealm(Authentication::Realm *realm);
    void addRealm(const QString &name, Authentication::Realm *realm, bool defaultRealm = true);

    void setUseSession(bool use);
    bool useSession() const;

    User authenticate(Context *ctx, const QString &username, const QString &password, const QString &realm = QString());
    User authenticate(Context *ctx, const CStringHash &userinfo, const QString &realm = QString());
    User findUser(Context *ctx, const CStringHash &userinfo, const QString &realm = QString());
    User user(Context *ctx);
    void setUser(Context *ctx, const User &user);
    bool userExists(Context *ctx);
    bool userInRealm(Context *ctx, const QString &realm);
    void logout(Context *ctx);

protected:
    void setAuthenticated(Context *ctx, const User &user, const QString &realmName);
    void persistUser(Context *ctx, const User &user, const QString &realmName);
    User restoreUser(Context *ctx, const QVariant &frozenUser, const QString &realmName);
    Realm* findRealmForPersistedUser(Context *ctx);

    AuthenticationPrivate *d_ptr;

private:
    friend class Credential;
    Q_DECLARE_PRIVATE(Authentication)
};

}

Q_DECLARE_METATYPE(CutelystPlugin::Authentication::User)
QDataStream &operator<<(QDataStream &out, const CutelystPlugin::Authentication::User &myObj);
QDataStream &operator>>(QDataStream &in, CutelystPlugin::Authentication::User &myObj);

#endif // AUTHENTICATION_H
