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

#ifndef AUTHENTICATIONREALM_H
#define AUTHENTICATIONREALM_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugins/authenticationuser.h>

namespace Cutelyst {

class Context;
class AuthenticationStore;
class AuthenticationCredential;
class CUTELYST_LIBRARY AuthenticationRealm : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationRealm(AuthenticationStore *store, AuthenticationCredential *credential, QObject *parent = 0);
    virtual ~AuthenticationRealm();

    AuthenticationStore *store() const;
    AuthenticationCredential *credential() const;

    virtual AuthenticationUser findUser(Context *c, const CStringHash &userinfo);
    virtual AuthenticationUser authenticate(Context *c, const CStringHash &authinfo);

protected:
    void removePersistedUser(Context *c);
    AuthenticationUser persistUser(Context *c, const AuthenticationUser &user);
    AuthenticationUser restoreUser(Context *c, const QVariant &frozenUser);
    QVariant userIsRestorable(Context *c);

private:
    friend class Authentication;
    friend class AuthenticationPrivate;

    AuthenticationStore *m_store;
    AuthenticationCredential *m_credential;
};

}

#endif // AUTHENTICATIONREALM_H
