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
#include "minimal.h"

#include <QVariant>

using namespace Cutelyst;

StoreMinimal::StoreMinimal(const QString &idField, QObject *parent) : AuthenticationStore(parent)
  , m_idField(idField)
{

}

StoreMinimal::~StoreMinimal()
{

}

void StoreMinimal::addUser(const AuthenticationUser &user)
{
    m_users << user;
}


AuthenticationUser StoreMinimal::findUser(Context *c, const ParamsMultiMap &userInfo)
{
    Q_UNUSED(c)
    AuthenticationUser ret;
    const QString id = userInfo.value(m_idField);

    const auto users = m_users;
    for (const AuthenticationUser &user : users) {
        if (user.id() == id) {
            ret = user;
            break;
        }
    }

    return ret;
}

QVariant StoreMinimal::forSession(Context *c, const AuthenticationUser &user)
{
    Q_UNUSED(c);
    return user.id();
}

AuthenticationUser StoreMinimal::fromSession(Context *c, const QVariant &frozenUser)
{
    return findUser(c, {
                        {m_idField, frozenUser.toString()}
                    });
}

#include "moc_minimal.cpp"
