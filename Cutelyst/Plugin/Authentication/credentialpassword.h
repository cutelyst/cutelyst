#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include <Cutelyst/Plugin/authentication.h>

namespace Cutelyst {
namespace Plugin {

class CredentialPassword : public Authentication::Credential
{
public:
    CredentialPassword();

    Authentication::User authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);

private:
    bool checkPassword(const Authentication::User &user, const CStringHash &authinfo);
};

} // namespace CutelystPlugin
}

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
