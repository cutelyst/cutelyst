#include "credentialpassword.h"

using namespace CutelystPlugin;

CredentialPassword::CredentialPassword()
{
}

Authentication::User CredentialPassword::authenticate(Cutelyst *c, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Authentication::User user = realm->findUser(c, authinfo);
}
