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

#include <Cutelyst/Plugin/plugin.h>

#include <QObject>

namespace Cutelyst {

namespace Plugin {

class AuthenticationPrivate;
class Authentication : public AbstractPlugin
{
    Q_OBJECT
public:
    class Realm;
    class User : public CStringHash
    {
    public:
        User();
        User(const QString &id);
        virtual ~User();

        /**
         * A unique ID by which a user can be retrieved from the store.
         */
        QString id() const;
        void setId(const QString &id);
        bool isNull() const;

        Realm *authRealm();
        void setAuthRealm(Realm *authRealm);

        virtual bool checkPassword(const QString &password) const;

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

    private:
        friend class Authentication;

        Store *m_store;
        Credential *m_credential;
    };

    explicit Authentication(QObject *parent = 0);
    ~Authentication();

    bool setup(Context *ctx);

    void addRealm(Authentication::Realm *realm);
    void addRealm(const QString &name, Authentication::Realm *realm, bool defaultRealm = true);

    User authenticate(const QString &username, const QString &password, const QString &realm = QString());
    User authenticate(const CStringHash &userinfo, const QString &realm = QString());
    User findUser(const CStringHash &userinfo, const QString &realm = QString());
    User user();
    void setUser(const User &user);
    bool userExists();
    bool userInRealm(const QString &realm);
    void logout();

protected:
    void setAuthenticated(const User &user, const QString &realmName);
    void persistUser(const User &user, const QString &realmName);
    User restoreUser(const QVariant &frozenUser, const QString &realmName);
    Realm* findRealmForPersistedUser();

    AuthenticationPrivate *d_ptr;

private:
    friend class Credential;
    Q_DECLARE_PRIVATE(Authentication)
};

}

}

Q_DECLARE_METATYPE(Cutelyst::Plugin::Authentication::User)
QDataStream &operator<<(QDataStream &out, const Cutelyst::Plugin::Authentication::User &myObj);
QDataStream &operator>>(QDataStream &in, Cutelyst::Plugin::Authentication::User &myObj);

#endif // AUTHENTICATION_H
