#ifndef AUTHENTICATION_STORE_MINIMAL_H
#define AUTHENTICATION_STORE_MINIMAL_H

#include "Plugin/plugin.h"
#include "Plugin/authentication.h"

namespace CutelystPlugin {

class StoreMinimal : public Authentication::Store
{
public:
    StoreMinimal();

    void addUser(const Authentication::User &user);

    Authentication::User findUser(Cutelyst *c, const CStringHash &userInfo);

    virtual QVariant forSession(Cutelyst *c, const Authentication::User &user);

    virtual Authentication::User fromSession(Cutelyst *c, const QVariant &frozenUser);

private:
    QList<Authentication::User> m_users;
};

} // namespace CutelystPlugin

#endif // AUTHENTICATION_STORE_MINIMAL_H
