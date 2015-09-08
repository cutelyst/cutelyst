/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "authenticationuser.h"

#include <QtCore/QDataStream>

using namespace Cutelyst;

AuthenticationUser::AuthenticationUser()
{

}

AuthenticationUser::AuthenticationUser(const QString &id) :
    m_id(id)
{

}

AuthenticationUser::~AuthenticationUser()
{
}

QString AuthenticationUser::id() const
{
    return m_id;
}

void AuthenticationUser::setId(const QString &id)
{
    m_id = id;
}

bool AuthenticationUser::isNull() const
{
    return m_id.isNull();
}

AuthenticationRealm *AuthenticationUser::authRealm()
{
    return m_realm;
}

void AuthenticationUser::setAuthRealm(AuthenticationRealm *authRealm)
{
    m_realm = authRealm;
}

bool AuthenticationUser::checkPassword(const QString &password) const
{
    Q_UNUSED(password)
    return false;
}

QDataStream &operator<<(QDataStream &out, const AuthenticationUser &user)
{
    out << user.id() << static_cast<CStringHash>(user);
    return out;
}

QDataStream &operator>>(QDataStream &in, AuthenticationUser &user)
{
    QString id;
    CStringHash hash;
    in >> id >> hash;
    user.setId(id);
    user.swap(hash);
    return in;
}
