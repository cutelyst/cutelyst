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
#include "authenticationstore.h"

#include <QVariant>

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

AuthenticationUser AuthenticationStore::autoCreateUser(Context *c, const ParamsMultiMap &userinfo) const
{
    Q_UNUSED(c)
    Q_UNUSED(userinfo)
    return AuthenticationUser();
}

bool AuthenticationStore::canAutoUpdateUser() const
{
    return false;
}

AuthenticationUser AuthenticationStore::autoUpdateUser(Context *c, const ParamsMultiMap &userinfo) const
{
    Q_UNUSED(c)
    Q_UNUSED(userinfo)
    return AuthenticationUser();
}

QVariant AuthenticationStore::forSession(Context *c, const AuthenticationUser &user)
{
    Q_UNUSED(c)
    return QVariant::fromValue(user);
}

AuthenticationUser AuthenticationStore::fromSession(Context *c, const QVariant &frozenUser)
{
    Q_UNUSED(c)
    return frozenUser.value<AuthenticationUser>();
}

#include "moc_authenticationstore.cpp"
