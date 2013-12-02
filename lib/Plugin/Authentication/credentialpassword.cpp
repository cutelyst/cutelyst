#include "credentialpassword.h"

#include <QDebug>

using namespace CutelystPlugin;

CredentialPassword::CredentialPassword()
{
}

Authentication::User CredentialPassword::authenticate(Cutelyst *c, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Authentication::User user = realm->findUser(c, authinfo);
    if (!user.isNull()) {
        if (checkPassword(user, authinfo)) {
            return user;
        }
        qDebug() << "Password didn't match";
    } else {
        qDebug() << "Unable to locate a user matching user info provided in realm";
    }
    return Authentication::User();
}

bool CredentialPassword::checkPassword(const Authentication::User &user, const CStringHash &authinfo)
{
    QString passwordType = "clear";

    QString password = authinfo.value("password");
    QString storedPassword = user.value("password");

    if (passwordType == "clear") {
        return password == storedPassword;
    } else {

    }
    return false;
}
