#include "authenticationrealm.h"

#include "authenticationstore.h"
#include "session.h"
#include "context.h"
#include "common.h"

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_AUTH_REALM, "cutelyst.plugin.authentication.realm")

AuthenticationRealm::AuthenticationRealm(AuthenticationStore *store, AuthenticationCredential *credential, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_credential(credential)
{

}

AuthenticationRealm::~AuthenticationRealm()
{

}

AuthenticationStore *AuthenticationRealm::store() const
{
    return m_store;
}

AuthenticationCredential *AuthenticationRealm::credential() const
{
    return m_credential;
}

AuthenticationUser AuthenticationRealm::findUser(Context *ctx, const CStringHash &userinfo)
{
    AuthenticationUser ret = m_store->findUser(ctx, userinfo);

    if (ret.isNull()) {
        if (m_store->canAutoCreateUser()) {
            ret = m_store->autoCreateUser(ctx, userinfo);
        }
    } else if (m_store->canAutoUpdateUser()) {
        ret = m_store->autoUpdateUser(ctx, userinfo);
    }

    return ret;
}

AuthenticationUser AuthenticationRealm::authenticate(Context *ctx, const CStringHash &authinfo)
{
    return m_credential->authenticate(ctx, this, authinfo);
}

void AuthenticationRealm::removePersistedUser(Context *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->deleteValue(c, QStringLiteral("Authentication::user"));
        session->deleteValue(c, QStringLiteral("Authentication::userRealm"));
    }
}

AuthenticationUser AuthenticationRealm::persistUser(Context *c, const AuthenticationUser &user)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        session->setValue(c,
                          QStringLiteral("Authentication::user"),
                          m_store->forSession(c, user));
    }

    return user;
}

AuthenticationUser AuthenticationRealm::restoreUser(Context *ctx, const QVariant &frozenUser)
{
    QVariant _frozenUser = frozenUser;
    if (_frozenUser.isNull()) {
        _frozenUser = userIsRestorable(ctx);
    }

    if (_frozenUser.isNull()) {
        return AuthenticationUser();
    }

    AuthenticationUser user = m_store->fromSession(ctx, _frozenUser);

    if (!user.isNull()) {
        ctx->plugin<Authentication*>()->setUser(ctx, user);

        // Sets the realm the user originated in
        user.setAuthRealm(this);
    } else {
        qCWarning(C_AUTH_REALM) << "Store claimed to have a restorable user, but restoration failed. Did you change the user's id_field?";
    }

    return user;
}

QVariant AuthenticationRealm::userIsRestorable(Context *c)
{
    Session *session = c->plugin<Session*>();
    if (session && session->isValid(c)) {
        return session->value(c, QStringLiteral("Authentication::user"));
    }
    return QVariant();
}
