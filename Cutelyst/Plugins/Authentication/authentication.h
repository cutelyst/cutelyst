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

/**
 * \ingroup plugins-authentication
 * \headerfile "" <Cutelyst/Plugins/Authentication/authentication.h>
 * \brief Abstract class to validate authentication credentials like user name and password.
 *
 * Use this class to create your own credential validator. Reimplement the pure virtual function
 * authenticate() in your derived class and set the object as \a credential to your
 * AuthenticationRealm. See CredentialPassword for an example implementation.
 *
 * %Cutelyst already ships some implementations to verify optionally PBKDF2 derived passwords from
 * \link CredentialPassword user input\endlink and \link CredentialHttp HTTP headers\endlink.
 *
 * For an example implementation see \ref plugins-authentication overview.
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationCredential : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new %AuthenticationCredential object with the given \a parent.
     */
    explicit AuthenticationCredential(QObject *parent = nullptr);

    /**
     * Destroys the %AuthenticationCredential object.
     */
    virtual ~AuthenticationCredential();

    /**
     * Tries to authenticate the \p authinfo using the given \p realm.
     * When reimplementing, get the credentials like user name and password provided by the
     * user from the \a authinfo and find the user data in the authentication \a realm with
     * AuthenticationRealm::findUser(). Than validate the user data and return a not null
     * AuthenticationUser in case off success.
     */
    virtual AuthenticationUser
        authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) = 0;
};

class AuthenticationPrivate;

/**
 * \ingroup plugins-authentication
 * \headerfile "" <Cutelyst/Plugins/Authentication/authentication.h>
 * \brief Main class to manage user authentication.
 *
 * The %Authentication class authenticates users against user data found in a specific realm.
 * You can add multiple \link AuthenticationRealm AuthenticationRealms\endlink identified by name
 * that consist of different AuthenticationStore and AuthenticationCredential providers. You could
 * for example add one realm for login on the website using a HTML formular and another one using
 * HTTP basic authentication to authenticate with your API routes.
 *
 * For an example implementation see \ref plugins-authentication overview.
 *
 * \par Logging category
 * cutelyst.plugin.authentication
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT Authentication : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Authentication)
public:
    /**
     * Default realm name.
     */
    static char *defaultRealm;

    /**
     * Constructs a new %Authentication object with the given \a parent.
     */
    Authentication(Application *parent);

    /**
     * Destroys the %Authentication object.
     */
    virtual ~Authentication() override;

    /**
     * Adds the \a realm.
     */
    void addRealm(std::shared_ptr<AuthenticationRealm> realm);

    /**
     * Creates a new AuthenticationRealm using \a store, \a credential and \a name.
     */
    void addRealm(std::shared_ptr<AuthenticationStore> store,
                  std::shared_ptr<AuthenticationCredential> credential,
                  const QString &name = QLatin1String(defaultRealm));

    /**
     * Returns an AuthenticationRealm object that was registered with \a name.
     */
    [[nodiscard]] std::shared_ptr<AuthenticationRealm>
        realm(const QString &name = QLatin1String(defaultRealm)) const;

    /**
     * Returns true if the \a userinfo could be validated against \a realm.
     */
    [[nodiscard]] static bool authenticate(Context *c,
                                           const ParamsMultiMap &userinfo,
                                           const QString &realm = QLatin1String(defaultRealm));

    /**
     * Returns \c true if the request information could be validated against \a realm.
     */
    [[nodiscard]] inline static bool
        authenticate(Context *c, const QString &realm = QLatin1String(defaultRealm));

    /**
     * Tries to find the user with \a userinfo using the \a realm, returning a non null
     * AuthenticationUser on success
     */
    [[nodiscard]] static AuthenticationUser
        findUser(Context *c,
                 const ParamsMultiMap &userinfo,
                 const QString &realm = QLatin1String(defaultRealm));

    /**
     * Returns the authenticated user if any, if you only need to know if the user is
     * authenticated (rather than retrieving it's ID) use userExists() instead which is faster.
     */
    [[nodiscard]] static AuthenticationUser user(Context *c);

    /**
     * Returns \c true if a user is logged in right now. The difference between
     * userExists() and user() is that userExists() will return \c true if a user is logged
     * in, even if it has not been yet retrieved from the storage backend. If you only
     * need to know if the user is logged in, depending on the storage mechanism this
     * can be much more efficient.
     * userExists() only looks into the session while user() is trying to restore the user.
     */
    [[nodiscard]] static bool userExists(Context *c);

    /**
     * Works like userExists(), except that it only returns \c true if a user is both logged
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
