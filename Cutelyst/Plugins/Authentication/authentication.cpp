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

const QStringView Authentication::defaultRealm      = u"cutelyst_authentication_default_realm";
const QStringView AuthenticationRealm::defaultRealm = u"cutelyst_authentication_default_realm";

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
    QStringView name)
{
    addRealm(std::make_shared<AuthenticationRealm>(store, credential, name));
}

std::shared_ptr<Cutelyst::AuthenticationRealm> Authentication::realm(QStringView name) const
{
    Q_D(const Authentication);
    return d->realms.value(name.toString());
}

bool Authentication::authenticate(Cutelyst::Context *c,
                                  const ParamsMultiMap &userinfo,
                                  QStringView realm)
{
    if (!auth) {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
        return false;
    }

    std::shared_ptr<AuthenticationRealm> realmPtr = auth->d_ptr->realm(realm);
    if (realmPtr) {
        const AuthenticationUser authUser = realmPtr->authenticate(c, userinfo);
        if (!authUser.isNull()) {
            AuthenticationPrivate::setAuthenticated(c, authUser, realm, realmPtr);
        }

        return !authUser.isNull();
    }

    qCWarning(C_AUTHENTICATION) << "Could not find realm" << realm;
    return false;
}

AuthenticationUser Authentication::findUser(Cutelyst::Context *c,
                                            const ParamsMultiMap &userinfo,
                                            QStringView realm)
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
        ret = AuthenticationPrivate::restoreUser(c, {}, {});
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

bool Authentication::userInRealm(Cutelyst::Context *c, QStringView realmName)
{
    const QVariant authUser = c->stash(AUTHENTICATION_USER);
    if (!authUser.isNull()) {
        return authUser.value<AuthenticationUser>().authRealm() == realmName;
    } else {
        if (!auth) {
            qCCritical(C_AUTHENTICATION, "Authentication plugin not registered!");
            return false;
        }

        auto realmPtr = AuthenticationPrivate::findRealmForPersistedUser(
            c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realmPtr) {
            return realmPtr->name() == realmName;
        } else {
            return false;
        }
    }
}

void Authentication::logout(Context *c)
{
    AuthenticationPrivate::setUser(c, AuthenticationUser());

    if (auth) {
        auto realmPtr = AuthenticationPrivate::findRealmForPersistedUser(
            c, auth->d_ptr->realms, auth->d_ptr->realmsOrder);
        if (realmPtr) {
            realmPtr->removePersistedUser(c);
        }
    } else {
        qCCritical(C_AUTHENTICATION) << "Authentication plugin not registered";
    }
}

bool Authentication::setup(Application *app)
{
    return connect(app, &Application::postForked, this, [this] { auth = this; });
}

std::shared_ptr<AuthenticationRealm> AuthenticationPrivate::realm(QStringView realmName) const
{
    return realms.value(QStringView{realmName.isNull() ? defaultRealm : realmName}.toString());
}

AuthenticationUser AuthenticationPrivate::restoreUser(Context *c,
                                                      const QVariant &frozenUser,
                                                      QStringView realmName)
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
        std::shared_ptr<AuthenticationRealm> realmPtr = realms.value(realmVariant.toString());
        if (realmPtr && !realmPtr->userIsRestorable(c).isNull()) {
            return realmPtr;
        }
    } else {
        // we have no choice but to ask each realm whether it has a persisted user.
        for (const auto &realmName : realmsOrder) {
            std::shared_ptr<AuthenticationRealm> realmPtr = realms.value(realmName);
            if (realmPtr && !realmPtr->userIsRestorable(c).isNull()) {
                return realmPtr;
            }
        }
    }

    return nullptr;
}

void AuthenticationPrivate::setAuthenticated(Context *c,
                                             const AuthenticationUser &user,
                                             QStringView realmName,
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
                                    QStringView realmName)
{
    if (user.isNull()) {
        c->setStash(AUTHENTICATION_USER, QVariant());
        c->setStash(AUTHENTICATION_USER_REALM, QVariant());
    } else {
        c->setStash(AUTHENTICATION_USER, QVariant::fromValue(user));
        c->setStash(AUTHENTICATION_USER_REALM, realmName.toString());
    }
}

void AuthenticationPrivate::persistUser(Context *c,
                                        const AuthenticationUser &user,
                                        QStringView realmName,
                                        std::shared_ptr<AuthenticationRealm> realm)
{
    if (Authentication::userInRealm(c, realmName)) {
        Session::setValue(c, AUTHENTICATION_USER_REALM, realmName.toString());

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
