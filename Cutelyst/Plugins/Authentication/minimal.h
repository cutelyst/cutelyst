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

class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreMinimal : public AuthenticationStore
{
    Q_OBJECT
public:
    /**
     * Constructs a new minimal authentication store object with the given parent.
     */
    explicit StoreMinimal(const QString &idField, QObject *parent = nullptr);
    virtual ~StoreMinimal() override;

    /**
     * Appends the user to internal memory storage
     */
    void addUser(const AuthenticationUser &user);

    /**
     * Reimplemented from AuthenticationStore::findUser().
     */
    virtual AuthenticationUser findUser(Context *c, const ParamsMultiMap &userInfo) final;

    /**
     * Reimplemented from AuthenticationStore::forSession().
     */
    virtual QVariant forSession(Context *c, const AuthenticationUser &user) final;

    /**
     * Reimplemented from AuthenticationStore::fromSession().
     */
    virtual AuthenticationUser fromSession(Context *c, const QVariant &frozenUser) final;

private:
    QString m_idField;
    QVector<AuthenticationUser> m_users;
};

} // namespace Cutelyst

#endif // AUTHENTICATION_STORE_MINIMAL_H
