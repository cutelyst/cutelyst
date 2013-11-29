#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include "Plugin/authentication.h"

namespace CutelystPlugin {

class CredentialPassword : public Authentication::Credential
{
public:
    CredentialPassword();

    Authentication::User authenticate(Cutelyst *c, Authentication::Realm *realm, const CStringHash &authinfo);
};

} // namespace CutelystPlugin

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
