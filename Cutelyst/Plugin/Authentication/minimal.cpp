#include "minimal.h"

using namespace Cutelyst::Plugin;

StoreMinimal::StoreMinimal()
{

}

void StoreMinimal::addUser(const Authentication::User &user)
{
    m_users << user;
}


Authentication::User StoreMinimal::findUser(Context *ctx, const CStringHash &userInfo)
{
    QString id = userInfo[QLatin1String("id")];
    if (id.isEmpty()) {
        id = userInfo[QLatin1String("username")];
    }

    Q_FOREACH (const Authentication::User &user, m_users) {
        if (user.id() == id) {
            return user;
        }
    }

    return Authentication::User();
}

QVariant StoreMinimal::forSession(Context *c, const Authentication::User &user)
{
    return user.id();
}

Authentication::User StoreMinimal::fromSession(Context *c, const QVariant &frozenUser)
{
    CStringHash userInfo;
    userInfo[QLatin1String("id")] = frozenUser.toString();
    return findUser(c, userInfo);
}
