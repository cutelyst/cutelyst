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

#include "authenticationstore.h"

using namespace Cutelyst;

AuthenticationStore::AuthenticationStore(QObject *parent) : QObject(parent)
{

}

AuthenticationStore::~AuthenticationStore()
{

}

bool AuthenticationStore::canAutoCreateUser() const
{
    return false;
}

AuthenticationUser AuthenticationStore::autoCreateUser(Context *ctx, const CStringHash &userinfo) const
{
    return AuthenticationUser();
}

bool AuthenticationStore::canAutoUpdateUser() const
{
    return false;
}

AuthenticationUser AuthenticationStore::autoUpdateUser(Context *ctx, const CStringHash &userinfo) const
{
    return AuthenticationUser();
}

QVariant AuthenticationStore::forSession(Context *ctx, const AuthenticationUser &user)
{
    Q_UNUSED(ctx)
    return QVariant::fromValue(user);
}

AuthenticationUser AuthenticationStore::fromSession(Context *ctx, const QVariant &frozenUser)
{
    Q_UNUSED(ctx)
    return frozenUser.value<AuthenticationUser>();
}
