/*
 * SPDX-FileCopyrightText: (C) 2014-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>

namespace Cutelyst {

/**
 * \ingroup plugins-authentication
 * \headerfile htpasswd.h <Cutelyst/Plugins/Authentication/htpasswd.h>
 * \brief %Authentication data store using a flat file.
 *
 * This authentication data store stores user data as a combination of <TT>username:password</TT>
 * in a flat file where each row contains one user. This is like the file created by Apache’s
 * <A HREF="https://httpd.apache.org/docs/current/programs/htpasswd.html">htpasswd</A> command.
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreHtpasswd : public AuthenticationStore
{
public:
    /**
     * Constructs a new %StoreHtpasswd object with the given file \a name.
     */
    StoreHtpasswd(const QString &name);

    /**
     * Destroys the %StoreHtpasswd object.
     */
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
