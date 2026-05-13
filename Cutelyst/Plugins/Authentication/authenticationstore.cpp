/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "authenticationstore.h"

#include <QVariant>

using namespace Cutelyst;

namespace Cutelyst {

bool AuthenticationStore::validatePassword(Context *c,
                                           const AuthenticationUser &user,
                                           const QString &password) const
{
    Q_UNUSED(c);
    Q_UNUSED(user);
    Q_UNUSED(password);
    return false;
}

} // namespace Cutelyst

AuthenticationStore::AuthenticationStore()
{
}

AuthenticationStore::~AuthenticationStore()
{
}

bool AuthenticationStore::canAutoCreateUser() const
{
    return false;
}

AuthenticationUser AuthenticationStore::autoCreateUser(Context *c,
                                                       const ParamsMultiMap &userinfo) const
{
    Q_UNUSED(c)
    Q_UNUSED(userinfo)
    return AuthenticationUser();
}

bool AuthenticationStore::canAutoUpdateUser() const
{
    return false;
}

AuthenticationUser AuthenticationStore::autoUpdateUser(Context *c,
                                                       const ParamsMultiMap &userinfo) const
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
