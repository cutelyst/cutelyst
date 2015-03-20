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

#ifndef AUTHENTICATION_STORE_MINIMAL_H
#define AUTHENTICATION_STORE_MINIMAL_H

#include <Cutelyst/Plugins/authenticationstore.h>

namespace Cutelyst {

class StoreMinimal : public AuthenticationStore
{
    Q_OBJECT
public:
    explicit StoreMinimal(QObject *parent = 0);
    virtual ~StoreMinimal();

    void addUser(const AuthenticationUser &user);

    AuthenticationUser findUser(Context *ctx, const CStringHash &userInfo);

    virtual QVariant forSession(Context *ctx, const AuthenticationUser &user);

    virtual AuthenticationUser fromSession(Context *ctx, const QVariant &frozenUser);

private:
    QList<AuthenticationUser> m_users;
};

} // namespace CutelystPlugin

#endif // AUTHENTICATION_STORE_MINIMAL_H
