/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authentication.h>

namespace Cutelyst {

/**
 * \ingroup plugins-authentication
 * \headerfile authenticationstore.h <Cutelyst/Plugins/Authentication/authenticationstore.h>
 * \brief Abstract class to retrieve user data from a store.
 *
 * Use this class to create your own user store. Reimplement the pure virtual function findUser()
 * to retrieve user data from a storage like a file or a database.
 *
 * For an example implementation see \ref plugins-authentication overview.
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationStore
{
public:
    /**
     * Constructs a new %AuthenticationStore object.
     */
    AuthenticationStore();

    /**
     * Destroys the %AuthenticationStore object.
     */
    virtual ~AuthenticationStore();

public:
    /**
     * Reimplement this if your store supports automatic user creation.
     */
    virtual bool canAutoCreateUser() const;

    /**
     * Reimplement this if your store supports
     * automatic user creation.
     */
    virtual AuthenticationUser autoCreateUser(Context *c, const ParamsMultiMap &userinfo) const;

    /**
     * Reimplement this if your store supports
     * automatic user update.
     */
    [[nodiscard]] virtual bool canAutoUpdateUser() const;

    /**
     * Reimplement this if your store supports
     * automatic user update.
     */
    virtual AuthenticationUser autoUpdateUser(Context *c, const ParamsMultiMap &userinfo) const;

    /**
     * Retrieve the user that matches the \a userinfo.
     */
    [[nodiscard]] virtual AuthenticationUser findUser(Context *c,
                                                      const ParamsMultiMap &userinfo) = 0;

    /**
     * Reimplement this so that you return a
     * serializable value that can be used to
     * identify the user.
     * The default implementation just returns
     * the user.
     */
    virtual QVariant forSession(Context *c, const AuthenticationUser &user);

    /**
     * Reimplement this so that you return a
     * User that was stored in the session.
     *
     * The default implementation just returns
     * the user.
     */
    [[nodiscard]] virtual AuthenticationUser fromSession(Context *c, const QVariant &frozenUser);
};

} // namespace Cutelyst
