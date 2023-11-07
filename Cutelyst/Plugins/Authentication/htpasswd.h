/*
 * SPDX-FileCopyrightText: (C) 2014-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreHtpasswd : public AuthenticationStore
{
public:
    /**
     * Constructs a new htpasswd store object with the given parent to represent the file with the
     * specified name.
     */
    StoreHtpasswd(const QString &name);
    virtual ~StoreHtpasswd() override;

    /**
     * Appends the user to htpasswd storage
     */
    void addUser(const ParamsMultiMap &user);

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
    QString m_filename;
};

} // namespace Cutelyst
