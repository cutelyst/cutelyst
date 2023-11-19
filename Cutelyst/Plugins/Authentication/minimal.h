/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATION_STORE_MINIMAL_H
#define AUTHENTICATION_STORE_MINIMAL_H

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>
#include <Cutelyst/cutelyst_global.h>

#include <QVector>

namespace Cutelyst {

/**
 * \ingroup plugins-authentication
 * \headerfile minimal.h <Cutelyst/Plugins/Authentication/minimal.h>
 * \brief Minimal in memory authentication data store.
 *
 * This authentication data store stores user data directly in memory. So it is gone when
 * the application stops or restarts.
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreMinimal : public AuthenticationStore
{
public:
    /**
     * Constructs a new %StoreMinimal object with the given \a idField.
     */
    explicit StoreMinimal(const QString &idField);

    /**
     * Destroys the %StoreMinimal object.
     */
    virtual ~StoreMinimal() override;

    /**
     * Appends the user to internal memory storage
     */
    void addUser(const AuthenticationUser &user);

    /**
     * Reimplemented from AuthenticationStore::findUser().
     */
    AuthenticationUser findUser(Context *c, const ParamsMultiMap &userInfo) override final;

    /**
     * Reimplemented from AuthenticationStore::forSession().
     */
    QVariant forSession(Context *c, const AuthenticationUser &user) override final;

    /**
     * Reimplemented from AuthenticationStore::fromSession().
     */
    AuthenticationUser fromSession(Context *c, const QVariant &frozenUser) override final;

private:
    QString m_idField;
    QVector<AuthenticationUser> m_users;
};

} // namespace Cutelyst

#endif // AUTHENTICATION_STORE_MINIMAL_H
