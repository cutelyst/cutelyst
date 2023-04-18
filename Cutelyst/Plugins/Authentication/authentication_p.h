/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef AUTHENTICATION_P_H
#define AUTHENTICATION_P_H

#include "authentication.h"

#include <QStringList>

namespace Cutelyst {

class AuthenticationRealm;
class AuthenticationPrivate
{
public:
    AuthenticationRealm *realm(const QString &realmName) const;

    static inline AuthenticationUser restoreUser(Context *c, const QVariant &frozenUser, const QString &realmName);
    static inline AuthenticationRealm *findRealmForPersistedUser(Cutelyst::Context *c, const QMap<QString, AuthenticationRealm *> &realms, const QStringList &realmsOrder);
    static inline void setAuthenticated(Context *c, const AuthenticationUser &user, const QString &realmName, AuthenticationRealm *realm);
    static inline void setUser(Context *c, const AuthenticationUser &user, const QString &realmName = QString());
    static inline void persistUser(Context *c, const AuthenticationUser &user, const QString &realmName, AuthenticationRealm *realm);

    QString defaultRealm;
    QMap<QString, AuthenticationRealm *> realms;
    QStringList realmsOrder;
};

} // namespace Cutelyst

#endif // AUTHENTICATION_P_H
