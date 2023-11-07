/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>
#include <Cutelyst/plugin.h>

namespace Cutelyst {

class Context;
class AuthenticationStore;
class AuthenticationRealm;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationCredential : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructs a new AuthenticationCredential object with the given parent.
     */
    explicit AuthenticationCredential(QObject *parent = nullptr);
    virtual ~AuthenticationCredential();

    /*!
     * Tries to authenticate the \p authinfo using the give \p realm.
     * Returns a not null AuthenticationUser object in case of success.
     */
    virtual AuthenticationUser
        authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) = 0;
};

class AuthenticationPrivate;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT Authentication : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Authentication)
public:
    /*! default realm name */
    static char *defaultRealm;

    /*!
     * Constructs a new Authentication object with the given parent.
     */
    Authentication(Application *parent);
    virtual ~Authentication() override;

    /*!
     * Adds the \p realm with \p name.
     * \note This class will take ownership of realm.
     */
    void addRealm(std::shared_ptr<AuthenticationRealm> realm);

    /*!
     * Creates a new AuthenticationRealm using \p store, \p credential and \p name to build it.
     * \note This class will take ownership of realm, store and credential.
     */
    void addRealm(std::shared_ptr<AuthenticationStore> store,
                  std::shared_ptr<AuthenticationCredential> credential,
                  const QString &name = QLatin1String(defaultRealm));

    /*!
     * Returns an AuthenticationRealm object that was registered with \p name.
     */
    [[nodiscard]] std::shared_ptr<AuthenticationRealm>
        realm(const QString &name = QLatin1String(defaultRealm)) const;

    /**
     * Returns true if the userinfo could be validated against a realm.
     */
    [[nodiscard]] static bool authenticate(Context *c,
                                           const ParamsMultiMap &userinfo,
                                           const QString &realm = QLatin1String(defaultRealm));

    /**
     * Returns true if the request information could be validated against a realm.
     */
    [[nodiscard]] inline static bool
        authenticate(Context *c, const QString &realm = QLatin1String(defaultRealm));

    /*!
     * Tries to find the user with \p userinfo using the \p realm, returning a non null
     * AuthenticationUser on success
     */
    [[nodiscard]] static AuthenticationUser
        findUser(Context *c,
                 const ParamsMultiMap &userinfo,
                 const QString &realm = QLatin1String(defaultRealm));

    /**
     * Returns the authenticated user if any, if you only need to know if the user is
     * authenticated (rather than retrieving it's ID) use userExists instead which is faster.
     */
    [[nodiscard]] static AuthenticationUser user(Context *c);

    /**
     * Returns true if a user is logged in right now. The difference between
     * userExists() and user() is that userExists will return true if a user is logged
     * in, even if it has not been yet retrieved from the storage backend. If you only
     * need to know if the user is logged in, depending on the storage mechanism this
     * can be much more efficient.
     * userExists() only looks into the session while user() is trying to restore the user.
     */
    [[nodiscard]] static bool userExists(Context *c);

    /**
     * Works like user_exists, except that it only returns true if a user is both logged
     * in right now and was retrieved from the realm provided.
     */
    [[nodiscard]] static bool userInRealm(Context *c,
                                          const QString &realmName = QLatin1String(defaultRealm));

    /**
     * Logs the user out. Deletes the currently logged in user from the Context and the session.
     * It does not delete the session.
     */
    static void logout(Context *c);

protected:
    virtual bool setup(Application *app) override;

    AuthenticationPrivate *d_ptr;
};

inline bool Authentication::authenticate(Context *c, const QString &realm)
{
    return Authentication::authenticate(c, ParamsMultiMap(), realm);
}

} // namespace Cutelyst

#endif // AUTHENTICATION_H
