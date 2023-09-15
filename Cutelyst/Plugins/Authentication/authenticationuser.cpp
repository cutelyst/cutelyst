/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "authenticationuser.h"

#include <QDataStream>
#include <QDebug>

using namespace Cutelyst;

AuthenticationUser::AuthenticationUser()
{
}

AuthenticationUser::AuthenticationUser(const QVariant &id)
{
    setId(id);
}

AuthenticationUser::~AuthenticationUser()
{
}

QVariant AuthenticationUser::id() const
{
    return m_data.value(QStringLiteral("id"));
}

void AuthenticationUser::setId(const QVariant &id)
{
    m_data.insert(QStringLiteral("id"), id);
}

bool AuthenticationUser::isNull() const
{
    return m_data.isEmpty();
}

QString AuthenticationUser::authRealm()
{
    return m_data.value(QStringLiteral("authRealm")).toString();
}

void AuthenticationUser::setAuthRealm(const QString &authRealm)
{
    m_data.insert(QStringLiteral("authRealm"), authRealm);
}

QDataStream &operator<<(QDataStream &out, const AuthenticationUser &user)
{
    out << user.data();
    return out;
}

QDataStream &operator>>(QDataStream &in, AuthenticationUser &user)
{
    QVariantMap map;
    in >> map;
    user.setData(map);
    return in;
}

QDebug operator<<(QDebug dbg, const AuthenticationUser &user)
{
    const QVariantMap map = user.data();
    const bool oldSetting = dbg.autoInsertSpaces();
    dbg.nospace() << "AuthenticationUser(";
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        dbg << '(' << it.key() << ", " << it.value() << ')';
    }
    dbg << ')';
    dbg.setAutoInsertSpaces(oldSetting);
    return dbg.maybeSpace();
}

#include "moc_authenticationuser.cpp"
