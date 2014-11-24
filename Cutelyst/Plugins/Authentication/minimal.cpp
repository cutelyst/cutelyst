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

#include "minimal.h"

using namespace Cutelyst;

StoreMinimal::StoreMinimal()
{

}

void StoreMinimal::addUser(const Authentication::User &user)
{
    m_users << user;
}


Authentication::User StoreMinimal::findUser(Context *ctx, const CStringHash &userInfo)
{
    Q_UNUSED(ctx)
    QString id = userInfo[QStringLiteral("id")];
    if (id.isEmpty()) {
        id = userInfo[QStringLiteral("username")];
    }

    Q_FOREACH (const Authentication::User &user, m_users) {
        if (user.id() == id) {
            return user;
        }
    }

    return Authentication::User();
}

QVariant StoreMinimal::forSession(Context *c, const Authentication::User &user)
{
    return user.id();
}

Authentication::User StoreMinimal::fromSession(Context *c, const QVariant &frozenUser)
{
    CStringHash userInfo;
    userInfo[QStringLiteral("id")] = frozenUser.toString();
    return findUser(c, userInfo);
}
