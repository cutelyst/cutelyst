#ifndef AUTHENTICATIONREALM_H
#define AUTHENTICATIONREALM_H

#include <Cutelyst/Plugins/authenticationuser.h>

namespace Cutelyst {

class AuthenticationStore;
class AuthenticationCredential;
class AuthenticationRealm : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationRealm(AuthenticationStore *store, AuthenticationCredential *credential, QObject *parent = 0);
    virtual ~AuthenticationRealm();

    AuthenticationStore *store() const;
    AuthenticationCredential *credential() const;

    virtual AuthenticationUser findUser(Context *ctx, const CStringHash &userinfo);
    virtual AuthenticationUser authenticate(Context *ctx, const CStringHash &authinfo);

protected:
    void removePersistedUser(Context *c);
    AuthenticationUser persistUser(Context *c, const AuthenticationUser &user);
    AuthenticationUser restoreUser(Context *c, const QVariant &frozenUser);
    QVariant userIsRestorable(Context *c);

private:
    friend class Authentication;

    AuthenticationStore *m_store;
    AuthenticationCredential *m_credential;
};

}

#endif // AUTHENTICATIONREALM_H
