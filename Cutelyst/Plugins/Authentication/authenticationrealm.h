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
    explicit AuthenticationRealm(std::shared_ptr<AuthenticationStore> store,
                                 std::shared_ptr<AuthenticationCredential> credential,
                                 const QString &name = QLatin1String(defaultRealm),
                                 QObject *parent     = nullptr);
    ~AuthenticationRealm() override;

    /*!
     * Returns the authentication store object
     */
    [[nodiscard]] std::shared_ptr<AuthenticationStore> store() const noexcept;

    /*!
     * Returns the authentication credential object
     */
    [[nodiscard]] std::shared_ptr<AuthenticationCredential> credential() const noexcept;

    /*!
     * Tries to find the user with \p authinfo returning a non null AuthenticationUser on success
     */
    [[nodiscard]] virtual AuthenticationUser findUser(Context *c, const ParamsMultiMap &userinfo);

    /*!
     * Tries to authenticate the user with \p authinfo returning a non null AuthenticationUser on
     * success
     */
    [[nodiscard]] virtual AuthenticationUser authenticate(Context *c,
                                                          const ParamsMultiMap &authinfo);

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
    [[nodiscard]] AuthenticationUser restoreUser(Context *c, const QVariant &frozenUser);

    /*!
     * Checks if user can be retrieved
     */
    [[nodiscard]] QVariant userIsRestorable(Context *c);

private:
    friend class Authentication;
    friend class AuthenticationPrivate;

    std::shared_ptr<AuthenticationStore> m_store;
    std::shared_ptr<AuthenticationCredential> m_credential;
};

} // namespace Cutelyst

#endif // AUTHENTICATIONREALM_H
