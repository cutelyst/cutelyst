/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "minimal.h"

#include <QVariant>

using namespace Cutelyst;

StoreMinimal::StoreMinimal(const QString &idField, QObject *parent)
    : AuthenticationStore(parent)
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
                           {m_idField, frozenUser.toString()},
                       });
}

#include "moc_minimal.cpp"
