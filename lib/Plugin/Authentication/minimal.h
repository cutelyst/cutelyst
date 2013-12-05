#ifndef AUTHENTICATION_STORE_MINIMAL_H
#define AUTHENTICATION_STORE_MINIMAL_H

#include "Plugin/plugin.h"
#include "Plugin/authentication.h"

namespace Cutelyst {

namespace CutelystPlugin {

class StoreMinimal : public Authentication::Store
{
public:
    StoreMinimal();

    void addUser(const Authentication::User &user);

    Authentication::User findUser(Context *ctx, const CStringHash &userInfo);

    virtual QVariant forSession(Context *ctx, const Authentication::User &user);

    virtual Authentication::User fromSession(Context *ctx, const QVariant &frozenUser);

private:
    QList<Authentication::User> m_users;
};

} // namespace CutelystPlugin

}

#endif // AUTHENTICATION_STORE_MINIMAL_H
