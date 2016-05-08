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

#ifndef AUTHENTICATIONUSER_H
#define AUTHENTICATIONUSER_H

#include <QDataStream>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>
#include <Cutelyst/plugin.h>

namespace Cutelyst {

class AuthenticationRealm;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationUser : public ParamsMultiMap
{
public:
    AuthenticationUser();
    AuthenticationUser(const QString &id);
    virtual ~AuthenticationUser();

    /**
     * A unique ID by which a AuthenticationUser can be retrieved from the store.
     */
    QString id() const;
    void setId(const QString &id);
    bool isNull() const;

    AuthenticationRealm *authRealm();
    void setAuthRealm(AuthenticationRealm *authRealm);

    virtual bool checkPassword(const QString &password) const;

private:
    QString m_id;
    AuthenticationRealm *m_realm;
};

}

Q_DECLARE_METATYPE(Cutelyst::AuthenticationUser)
QDataStream &operator<<(QDataStream &out, const Cutelyst::AuthenticationUser &myObj);
QDataStream &operator>>(QDataStream &in, Cutelyst::AuthenticationUser &myObj);

#endif // AUTHENTICATIONUSER_H
