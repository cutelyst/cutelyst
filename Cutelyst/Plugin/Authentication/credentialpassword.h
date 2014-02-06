#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include <Cutelyst/Plugin/authentication.h>
#include <QCryptographicHash>

namespace Cutelyst {
namespace Plugin {

class CredentialPassword : public Authentication::Credential
{
public:
    enum Type {
        None,
        Clear,
        Hashed,
        SelfCheck
    };
    CredentialPassword();

    Authentication::User authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo);

    QString passwordField() const;
    void setPasswordField(const QString &fieldName);

    Type passwordType() const;
    void setPasswordType(Type type);

    QCryptographicHash::Algorithm hashType() const;
    void setHashType(QCryptographicHash::Algorithm type);

    QString passwordPreSalt() const;
    void setPasswordPreSalt(const QString &passwordPreSalt);

    QString passwordPostSalt() const;
    void setPasswordPostSalt(const QString &passwordPostSalt);

private:
    bool checkPassword(const Authentication::User &user, const CStringHash &authinfo);

    QString m_passwordField;
    Type m_passwordType;
    QCryptographicHash::Algorithm m_hashType;
    QString m_passwordPreSalt;
    QString m_passwordPostSalt;
};

} // namespace Plugin
}

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
