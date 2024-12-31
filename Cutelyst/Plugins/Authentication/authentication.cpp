/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "authentication_p.h"
#include "authenticationrealm.h"
#include "authenticationstore.h"
#include "context.h"

#include <Cutelyst/Plugins/Session/session.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(C_AUTHENTICATION, "cutelyst.plugin.authentication", QtWarningMsg)

using namespace Cutelyst;
using namespace Qt::StringLiterals;

char *Authentication::defaultRealm = const_cast<char *>("cutelyst_authentication_default_realm");
char *AuthenticationRealm::defaultRealm =
    const_cast<char *>("cutelyst_authentication_default_realm");
namespace {
thread_local Authentication *auth = nullptr;

const auto AUTHENTICATION_USER       = u"_c_authentication_user"_s;
const auto AUTHENTICATION_USER_REALM = u"_c_authentication_user_realm"_s;
} // namespace

Authentication::Authentication(Application *parent)
    : Plugin(parent)
    , d_ptr(new AuthenticationPrivate)
{
    qRegisterMetaType<AuthenticationUser>();
}

Authentication::~Authentication()
{
    delete d_ptr;
}

void Authentication::addRealm(std::shared_ptr<Cutelyst::AuthenticationRealm> realm)
{
    Q_D(Authentication);
    realm->setParent(nullptr);
    d->realms.insert(realm->objectName(), realm);
    d->realmsOrder.append(realm->objectName());
}

void Cutelyst::Authentication::addRealm(
    std::shared_ptr<Cutelyst::AuthenticationStore> store,
    std::shared_ptr<Cutelyst::AuthenticationCredential> credential,
    const QString &name)
{
    addRealm(std::make_shared<AuthenticationRealm>(store, credential, name));
}

std::shared_ptr<Cutelyst::AuthenticationRealm> Authentication::realm(const QString &name) const
{
    Q_D(const Authentication);
    return d->realms.value(name);
}

bool Authentication::authenticate(Cutelyst::Context *c,
                                  const ParamsMultiMap &userinfo,
                                  const QString &realm)
{
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return false;
    }

    std::shared_ptr<AuthenticationRealm> realmPtr = auth->d_ptr->realm(realm);
    if (realmPtr) {
        const AuthenticationUser user = realmPtr->authenticate(c, userinfo);
        if (!user.isNull()) {
            AuthenticationPrivate::setAuthenticated(c, user, realm, realmPtr);
        }

        return !user.isNull();
    }

    qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
    return false;
}

AuthenticationUser Authentication::findUser(Cutelyst::Context *c,
                                            const ParamsMultiMap &userinfo,
                                            const QString &realm)
{
    AuthenticationUser ret;
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return ret;
    }

    auto realmPtr = auth->d_ptr->realm(realm);
    if (!realmPtr) {
        qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
        return ret;
    }

    ret = realmPtr->findUser(c, userinfo);
    return ret;
}

Cutelyst::AuthenticationUser Authentication::user(Cutelyst::Context *c)
{
    AuthenticationUser ret;
    const QVariant user = c->stash(AUTHENTICATION_USER);
    if (user.isNull()) {
        ret = AuthenticationPrivate::restoreUser(c, QVariant(), QString());
    } else {
        ret = user.value<AuthenticationUser>();
    }
    return ret;
}

bool Authentication::userExists(Cutelyst::Context *c)
{
    if (!c->stash(AUTHENTICATION_USER).isNull()) {
        return true;
    } else {
        if (auth) {
            if (AuthenticationPrivate::findRealmForPersistedUser(
                    c, auth->d_ptr->realms, auth->d_ptr->realmsOrder)) {
                return true;
            }
        } else {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
        }
        return false;
    }
}

bool Authentication::userInRealm(Cutelyst::Context *c, const QString &realmName)
{
    const QVariant user = c->stash(AUTHENTICATION_USER);
    if (!user.isNull()) {
        return user.value<AuthenticationUser>().authRealm() == realmName;
    } else {
        if (!auth) {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
            return false;
        }

        auto realm = AuthenticationPrivate::findRealmForPersistedUser(
            c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realm) {
            return realm->name() == realmName;
        } else {
            return false;
        }
    }
}

void Authentication::logout(Context *c)
{
    AuthenticationPrivate::setUser(c, AuthenticationUser());

    if (auth) {
        auto realm = AuthenticationPrivate::findRealmForPersistedUser(
            c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realm) {
            realm->removePersistedUser(c);
        }
    } else {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
    }
}

bool Authentication::setup(Application *app)
{
    return connect(app, &Application::postForked, this, [this] { auth = this; });
}

std::shared_ptr<AuthenticationRealm> AuthenticationPrivate::realm(const QString &realmName) const
{
    return realms.value(realmName.isNull() ? defaultRealm : realmName);
}

AuthenticationUser AuthenticationPrivate::restoreUser(Context *c,
                                                      const QVariant &frozenUser,
                                                      const QString &realmName)
{
    AuthenticationUser ret;
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return ret;
    }

    auto realmPtr = auth->d_ptr->realm(realmName);
    if (!realmPtr) {
        realmPtr = AuthenticationPrivate::findRealmForPersistedUser(
            c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
    }

    if (!realmPtr) {
        return ret;
    }

    ret = realmPtr->restoreUser(c, frozenUser);

    AuthenticationPrivate::setUser(c, ret);

    return ret;
}

std::shared_ptr<AuthenticationRealm> AuthenticationPrivate::findRealmForPersistedUser(
    Context *c,
    const QMap<QString, std::shared_ptr<AuthenticationRealm>> &realms,
    const QStringList &realmsOrder)
{
    const QVariant realmVariant = Session::value(c, AUTHENTICATION_USER_REALM);
    if (!realmVariant.isNull()) {
        std::shared_ptr<AuthenticationRealm> realm = realms.value(realmVariant.toString());
        if (realm && !realm->userIsRestorable(c).isNull()) {
            return realm;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        for (const QString &realmName : realmsOrder) {
            std::shared_ptr<AuthenticationRealm> realm = realms.value(realmName);
            if (realm && !realm->userIsRestorable(c).isNull()) {
                return realm;
            }
        }
    }

    return nullptr;
}

void AuthenticationPrivate::setAuthenticated(Context *c,
                                             const AuthenticationUser &user,
                                             const QString &realmName,
                                             std::shared_ptr<AuthenticationRealm> realm)
{
    AuthenticationPrivate::setUser(c, user, realmName);

    if (!realm) {
        qCWarning(C_AUTHENTICATION) << "Called with invalid realm" << realmName;
    }

    AuthenticationPrivate::persistUser(c, user, realmName, realm);
}

void AuthenticationPrivate::setUser(Context *c,
                                    const AuthenticationUser &user,
                                    const QString &realmName)
{
    if (user.isNull()) {
        c->setStash(AUTHENTICATION_USER, QVariant());
        c->setStash(AUTHENTICATION_USER_REALM, QVariant());
    } else {
        c->setStash(AUTHENTICATION_USER, QVariant::fromValue(user));
        c->setStash(AUTHENTICATION_USER_REALM, realmName);
    }
}

void AuthenticationPrivate::persistUser(Context *c,
                                        const AuthenticationUser &user,
                                        const QString &realmName,
                                        std::shared_ptr<AuthenticationRealm> realm)
{
    if (Authentication::userInRealm(c, realmName)) {
        Session::setValue(c, AUTHENTICATION_USER_REALM, realmName);

        if (realm) {
            realm->persistUser(c, user);
        }
    }
}

Cutelyst::AuthenticationCredential::AuthenticationCredential(QObject *parent)
    : QObject(parent)
{
}

Cutelyst::AuthenticationCredential::~AuthenticationCredential()
{
}

#include "moc_authentication.cpp"
