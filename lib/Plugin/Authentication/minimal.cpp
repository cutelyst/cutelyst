#include "minimal.h"

using namespace CutelystPlugin;

StoreMinimal::StoreMinimal()
{

}

void StoreMinimal::addUser(const Authentication::User &user)
{
    m_users << user;
}


Authentication::User CutelystPlugin::StoreMinimal::findUser(Cutelyst *c, const CutelystPlugin::CStringHash &userinfo)
{
    QString id = userinfo[QLatin1String("id")];
    if (id.isEmpty()) {
        id = userinfo[QLatin1String("username")];
    }

    foreach (const Authentication::User &user, m_users) {
        if (user.id() == id) {
            return user;
        }
    }

    return Authentication::User();
}
