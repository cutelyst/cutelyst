/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "authenticationrealm.h"

#include "authenticationstore.h"
#include "common.h"
#include "context.h"

#include <Cutelyst/Plugins/Session/session.h>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_AUTH_REALM, "cutelyst.plugin.authentication.realm", QtWarningMsg)

#define SESSION_AUTHENTICATION_USER "__authentication_user"
#define SESSION_AUTHENTICATION_USER_REALM "__authentication_user_realm" // in authentication.cpp

AuthenticationRealm::AuthenticationRealm(std::shared_ptr<AuthenticationStore> store,
                                         std::shared_ptr<AuthenticationCredential> credential,
                                         const QString &name,
                                         QObject *parent)
    : Component(parent)
    , m_store(store)
    , m_credential(credential)
{
    m_credential->setParent(this);
    setObjectName(name);
    setName(name);
}

AuthenticationRealm::~AuthenticationRealm()
{
}

std::shared_ptr<AuthenticationStore> AuthenticationRealm::store() const noexcept
{
    return m_store;
}

std::shared_ptr<AuthenticationCredential> AuthenticationRealm::credential() const noexcept
{
    return m_credential;
}

AuthenticationUser AuthenticationRealm::findUser(Context *c, const ParamsMultiMap &userinfo)
{
    AuthenticationUser ret = m_store->findUser(c, userinfo);

    if (ret.isNull()) {
        if (m_store->canAutoCreateUser()) {
            ret = m_store->autoCreateUser(c, userinfo);
        }
    } else {
        if (m_store->canAutoUpdateUser()) {
            ret = m_store->autoUpdateUser(c, userinfo);
        }
    }

    if (!ret.isNull() && ret.authRealm() != name()) {
        ret.setAuthRealm(name());
    }

    return ret;
}

AuthenticationUser AuthenticationRealm::authenticate(Context *c, const ParamsMultiMap &authinfo)
{
    return m_credential->authenticate(c, this, authinfo);
}

void AuthenticationRealm::removePersistedUser(Context *c)
{
    Session::deleteValues(c,
                          {QStringLiteral(SESSION_AUTHENTICATION_USER),
                           QStringLiteral(SESSION_AUTHENTICATION_USER_REALM)});
}

AuthenticationUser AuthenticationRealm::persistUser(Context *c, const AuthenticationUser &user)
{
    Session::setValue(c, QStringLiteral(SESSION_AUTHENTICATION_USER), m_store->forSession(c, user));
    Session::setValue(c, QStringLiteral(SESSION_AUTHENTICATION_USER_REALM), objectName());

    return user;
}

AuthenticationUser AuthenticationRealm::restoreUser(Context *c, const QVariant &frozenUser)
{
    AuthenticationUser user;
    QVariant _frozenUser = frozenUser;
    if (_frozenUser.isNull()) {
        _frozenUser = userIsRestorable(c);
    }

    if (_frozenUser.isNull()) {
        return user;
    }

    user = m_store->fromSession(c, _frozenUser);

    if (!user.isNull()) {
        // Sets the realm the user originated in
        user.setAuthRealm(objectName());
    } else {
        qCWarning(C_AUTH_REALM) << "Store claimed to have a restorable user, but restoration "
                                   "failed. Did you change the user's id_field?";
    }

    return user;
}

QVariant AuthenticationRealm::userIsRestorable(Context *c)
{
    // No need to check if session is valid
    // as ::value will do that for us
    return Session::value(c, QStringLiteral(SESSION_AUTHENTICATION_USER));
}

#include "moc_authenticationrealm.cpp"
