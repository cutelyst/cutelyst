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

#ifndef AUTHENTICATION_STORE_H
#define AUTHENTICATION_STORE_H

#include <Cutelyst/Plugins/authentication.h>

namespace Cutelyst {

class AuthenticationStore : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationStore(QObject *parent = 0);
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
    virtual Authentication::User autoCreateUser(Context *ctx, const CStringHash &userinfo) const;

    /**
     * Reimplement this if your store supports
     * automatic user update
     */
    virtual bool canAutoUpdateUser() const;

    /**
     * Reimplement this if your store supports
     * automatic user update
     */
    virtual Authentication::User autoUpdateUser(Context *ctx, const CStringHash &userinfo) const;

    /**
     * Retrieve the user that matches the user info
     */
    virtual Authentication::User findUser(Context *ctx, const CStringHash &userinfo) = 0;

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
    virtual Authentication::User fromSession(Context *ctx, const QVariant &frozenUser);
};

}

#endif // AUTHENTICATIONSTORE_H
