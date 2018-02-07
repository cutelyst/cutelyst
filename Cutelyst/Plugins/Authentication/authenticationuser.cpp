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
#include "authenticationuser.h"

#include <QtCore/QDataStream>

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
    return m_data.value(QStringLiteral("id")).toString();
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
