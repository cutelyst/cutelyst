/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include <Cutelyst/plugin.h>

namespace Cutelyst {

class AuthenticationStore;
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

    class Realm
    {
    public:
        Realm(AuthenticationStore *store, Credential *credential);

        AuthenticationStore *store() const;
        Credential *credential() const;

        virtual User findUser(Context *ctx, const CStringHash &userinfo);
        virtual User authenticate(Context *ctx, const CStringHash &authinfo);

    protected:
        void removePersistedUser(Context *ctx);
        User persistUser(Context *ctx, const Authentication::User &user);
        User restoreUser(Context *ctx, const QVariant &frozenUser);
        QVariant userIsRestorable(Context *ctx);

    private:
        friend class Authentication;

        AuthenticationStore *m_store;
        Credential *m_credential;
    };

    static char *defaultRealm;

    explicit Authentication(QObject *parent = 0);
    ~Authentication();

    bool setup(Context *ctx);

    void addRealm(Authentication::Realm *realm, const QString &name = QLatin1String(defaultRealm));

    Authentication::Realm *realm(const QString &name = QLatin1String(defaultRealm)) const;

    User authenticate(const QString &username, const QString &password, const QString &realm = QLatin1String(defaultRealm));
    User authenticate(const CStringHash &userinfo = CStringHash(), const QString &realm = QLatin1String(defaultRealm));
    User findUser(const CStringHash &userinfo, const QString &realm = QLatin1String(defaultRealm));
    User user();
    void setUser(const User &user);
    bool userExists();
    bool userInRealm(const QString &realm = QLatin1String(defaultRealm));
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

Q_DECLARE_METATYPE(Cutelyst::Authentication::User)
QDataStream &operator<<(QDataStream &out, const Cutelyst::Authentication::User &myObj);
QDataStream &operator>>(QDataStream &in, Cutelyst::Authentication::User &myObj);

#endif // AUTHENTICATION_H
