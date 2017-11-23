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
#ifndef AUTHENTICATION_STORE_H
#define AUTHENTICATION_STORE_H

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationStore : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new authentication store object with the given parent.
     */
    explicit AuthenticationStore(QObject *parent = nullptr);
    virtual ~AuthenticationStore();

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
    virtual AuthenticationUser autoCreateUser(Context *c, const ParamsMultiMap &userinfo) const;

    /**
     * Reimplement this if your store supports
     * automatic user update
     */
    virtual bool canAutoUpdateUser() const;

    /**
     * Reimplement this if your store supports
     * automatic user update
     */
    virtual AuthenticationUser autoUpdateUser(Context *c, const ParamsMultiMap &userinfo) const;

    /**
     * Retrieve the user that matches the user info
     */
    virtual AuthenticationUser findUser(Context *c, const ParamsMultiMap &userinfo) = 0;

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
    virtual AuthenticationUser fromSession(Context *c, const QVariant &frozenUser);
};

}

#endif // AUTHENTICATIONSTORE_H
