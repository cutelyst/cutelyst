/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATIONREALM_H
#define AUTHENTICATIONREALM_H

#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/component.h>
#include <Cutelyst/cutelyst_global.h>

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
    virtual ~AuthenticationRealm() override;

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

} // namespace Cutelyst

#endif // AUTHENTICATIONREALM_H
