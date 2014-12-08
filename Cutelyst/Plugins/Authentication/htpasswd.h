/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef AUTHENTICATION_STORE_HTPASSWD_H
#define AUTHENTICATION_STORE_HTPASSWD_H

#include <Cutelyst/Plugins/authenticationstore.h>

namespace Cutelyst {

class StoreHtpasswd : public AuthenticationStore
{
public:
    StoreHtpasswd(const QString &file);
    virtual ~StoreHtpasswd();

    void addUser(const CStringHash &user);

    virtual Authentication::User findUser(Context *ctx, const CStringHash &userInfo);

    virtual QVariant forSession(Context *ctx, const Authentication::User &user);

    virtual Authentication::User fromSession(Context *ctx, const QVariant &frozenUser);
};

}

#endif // AUTHENTICATION_STORE_HTPASSWD_H
