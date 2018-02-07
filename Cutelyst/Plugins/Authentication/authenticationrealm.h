/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef AUTHENTICATIONREALM_H
#define AUTHENTICATIONREALM_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/component.h>

namespace Cutelyst {

class Context;
class AuthenticationStore;
class AuthenticationCredential;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationRealm : public Component
{
    Q_OBJECT
public:
    /*! default realm name */
    static char *defaultRealm;

    /*!
     * Constructs a new AuthenticationRealm object with the given parent.
     * \note This class will take ownership of store and credential.
     */
    explicit AuthenticationRealm(AuthenticationStore *store, AuthenticationCredential *credential, const QString &name = QLatin1String(defaultRealm), QObject *parent = nullptr);
    virtual ~AuthenticationRealm();

    /*!
     * Returns the authentication store object
     */
    AuthenticationStore *store() const;

    /*!
     * Returns the authentication credential object
     */
    AuthenticationCredential *credential() const;

    /*!
     * Tries to find the user with \p authinfo returning a non null AuthenticationUser on success
     */
    virtual AuthenticationUser findUser(Context *c, const ParamsMultiMap &userinfo);

    /*!
     * Tries to authenticate the user with \p authinfo returning a non null AuthenticationUser on success
     */
    virtual AuthenticationUser authenticate(Context *c, const ParamsMultiMap &authinfo);

    /*!
     * Removes the user from the session
     */
    void removePersistedUser(Context *c);

    /*!
     * Stores the user on the session
     */
    AuthenticationUser persistUser(Context *c, const AuthenticationUser &user);

    /*!
     * Retrieves the user from the store
     */
    AuthenticationUser restoreUser(Context *c, const QVariant &frozenUser);

    /*!
     * Checks if user can be retrieved
     */
    QVariant userIsRestorable(Context *c);

private:
    friend class Authentication;
    friend class AuthenticationPrivate;

    AuthenticationStore *m_store;
    AuthenticationCredential *m_credential;
};

}

#endif // AUTHENTICATIONREALM_H
