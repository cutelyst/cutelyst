/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATION_STORE_HTPASSWD_H
#define AUTHENTICATION_STORE_HTPASSWD_H

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreHtpasswd : public AuthenticationStore
{
    Q_OBJECT
public:
    /**
     * Constructs a new htpasswd store object with the given parent to represent the file with the specified name.
     */
    explicit StoreHtpasswd(const QString &name, QObject *parent = nullptr);
    virtual ~StoreHtpasswd() override;

    /**
     * Appends the user to htpasswd storage
     */
    void addUser(const ParamsMultiMap &user);

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
    QString m_filename;
};

} // namespace Cutelyst

#endif // AUTHENTICATION_STORE_HTPASSWD_H
