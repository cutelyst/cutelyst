/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "minimal.h"

#include <algorithm>

#include <QVariant>

using namespace Cutelyst;

StoreMinimal::StoreMinimal(const QString &idField)
    : m_idField(idField)
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
    const QString id = userInfo.value(m_idField);

    auto it = std::ranges::find_if(
        m_users, [id](const AuthenticationUser &user) { return user.id() == id; });

    if (it != m_users.end()) {
        return *it;
    }

    return {};
}

QVariant StoreMinimal::forSession(Context *c, const AuthenticationUser &user)
{
    Q_UNUSED(c);
    return user.id();
}

AuthenticationUser StoreMinimal::fromSession(Context *c, const QVariant &frozenUser)
{
    return findUser(c,
                    {
                        {m_idField, frozenUser.toString()},
                    });
}
