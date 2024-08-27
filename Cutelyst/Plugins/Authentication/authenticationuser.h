/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATIONUSER_H
#define AUTHENTICATIONUSER_H

#include <Cutelyst/Plugins/authentication_export.h>
#include <Cutelyst/paramsmultimap.h>
#include <Cutelyst/plugin.h>

#include <QDataStream>
#include <QVariantMap>

namespace Cutelyst {

class AuthenticationRealm;

/**
 * \ingroup plugins-authentication
 * \headerfile authenticationuser.h <Cutelyst/Plugins/Authentication/authenticationuser.h>
 * \brief Container for user data retrieved from an AuthenticationStore.
 *
 * Create this object in your reimplementation of AuthenticationStore::findUser() and populate
 * it with the data retrieved from your store.
 *
 * For an example implementation see \ref plugins-authentication overview.
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT AuthenticationUser
{
    Q_GADGET
public:
    /**
     * Constructs a new %AuthenticationUser object.
     */
    AuthenticationUser();

    /**
     * Constructs a new %AuthenticationUser object with the given \a id.
     */
    AuthenticationUser(const QVariant &id);

    /**
     * Destroys the %AuthenticationUser object.
     */
    virtual ~AuthenticationUser();

    /**
     * A unique ID by which a %AuthenticationUser can be retrieved from the store.
     */
    [[nodiscard]] QVariant id() const;

    /**
     * Sets the unique user \a id restored from the store
     */
    void setId(const QVariant &id);

    /**
     * Returns \c true if the object is null.
     */
    [[nodiscard]] bool isNull() const;

    /**
     * Returns the authentication realm from which this user was retrieved.
     */
    [[nodiscard]] QString authRealm();

    /**
     * Sets the authentication realm from which this user was retrieved.
     */
    void setAuthRealm(const QString &authRealm);

    /**
     * Returns the internal data object.
     */
    [[nodiscard]] inline QVariantMap data() const;

    /**
     * Directly sets the internal \a data object.
     */
    inline void setData(const QVariantMap &data);

    /**
     * Inserts a new item with the key \a key and a value of \a value.
     *
     * If there is already an item with the key \a key, that item's value is replaced with
     * \a value.
     */
    inline void insert(const QString &key, const QVariant &value);

    /**
     * Returns the value associated with the \a key key.
     *
     * If the internal map contains no item with key \a key, the function returns \a defaultValue.
     * If no \a defaultValue is specified, the function returns a default-constructed value.
     */
    [[nodiscard]] inline QVariant value(const QString &key,
                                        const QVariant &defaultValue = QVariant()) const;

    inline operator QVariant() const { return QVariant::fromValue(m_data); }

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

QDebug CUTELYST_PLUGIN_AUTHENTICATION_EXPORT operator<<(QDebug dbg,
                                                        const Cutelyst::AuthenticationUser &user);

#endif // AUTHENTICATIONUSER_H
