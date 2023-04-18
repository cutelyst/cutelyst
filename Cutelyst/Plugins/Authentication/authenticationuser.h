/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATIONUSER_H
#define AUTHENTICATIONUSER_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>
#include <Cutelyst/plugin.h>

#include <QDataStream>
#include <QVariantMap>

namespace Cutelyst {

class AuthenticationRealm;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationUser
{
    Q_GADGET
public:
    /*!
     * Constructs a new AuthenticationUser object
     */
    AuthenticationUser();

    /*!
     * Constructs a new AuthenticationUser object with the given id
     */
    AuthenticationUser(const QVariant &id);
    virtual ~AuthenticationUser();

    /**
     * A unique ID by which a AuthenticationUser can be retrieved from the store.
     */
    QVariant id() const;

    /*!
     * Sets the unique user id restored from the store
     */
    void setId(const QVariant &id);

    /*!
     * Returns true if the object is null
     */
    bool isNull() const;

    /*!
     * Returns the authentication realm from which this user was retrieved
     */
    QString authRealm();

    /*!
     * Sets the authentication realm from which this user was retrieved
     */
    void setAuthRealm(const QString &authRealm);

    inline QVariantMap data() const;

    inline void setData(const QVariantMap &data);

    inline void insert(const QString &key, const QVariant &value);

    inline QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    inline operator QVariant() const
    {
        return QVariant::fromValue(m_data);
    }

protected:
    QVariantMap m_data;
};

QVariantMap AuthenticationUser::data() const
{
    return m_data;
}

void AuthenticationUser::setData(const QVariantMap &data)
{
    m_data = data;
}

void AuthenticationUser::insert(const QString &key, const QVariant &value)
{
    m_data.insert(key, value);
}

QVariant AuthenticationUser::value(const QString &key, const QVariant &defaultValue) const
{
    return m_data.value(key, defaultValue);
}

} // namespace Cutelyst

Q_DECLARE_METATYPE(Cutelyst::AuthenticationUser)
QDataStream &operator<<(QDataStream &out, const Cutelyst::AuthenticationUser &myObj);
QDataStream &operator>>(QDataStream &in, Cutelyst::AuthenticationUser &myObj);

QDebug CUTELYST_PLUGIN_AUTHENTICATION_EXPORT operator<<(QDebug dbg, const Cutelyst::AuthenticationUser &user);

#endif // AUTHENTICATIONUSER_H
