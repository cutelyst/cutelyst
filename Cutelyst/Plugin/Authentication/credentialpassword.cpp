#include "credentialpassword.h"

#include <QDebug>

using namespace Cutelyst::Plugin;

CredentialPassword::CredentialPassword() :
    m_passwordField(QLatin1String("password")),
    m_passwordType(None),
    m_hashType(QCryptographicHash::Md5)
{
}

Authentication::User CredentialPassword::authenticate(Context *ctx, Authentication::Realm *realm, const CStringHash &authinfo)
{
    Authentication::User user = realm->findUser(ctx, authinfo);
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

QString CredentialPassword::passwordField() const
{
    return m_passwordField;
}

void CredentialPassword::setPasswordField(const QString &fieldName)
{
    m_passwordField = fieldName;
}

CredentialPassword::Type CredentialPassword::passwordType() const
{
    return m_passwordType;
}

void CredentialPassword::setPasswordType(CredentialPassword::Type type)
{
    m_passwordType = type;
}

QCryptographicHash::Algorithm CredentialPassword::hashType() const
{
    return m_hashType;
}

void CredentialPassword::setHashType(QCryptographicHash::Algorithm type)
{
    m_hashType = type;
}

QString CredentialPassword::passwordPreSalt() const
{
    return m_passwordPreSalt;
}

void CredentialPassword::setPasswordPreSalt(const QString &passwordPreSalt)
{
    m_passwordPreSalt = passwordPreSalt;
}

QString CredentialPassword::passwordPostSalt() const
{
    return m_passwordPostSalt;
}

void CredentialPassword::setPasswordPostSalt(const QString &passwordPostSalt)
{
    m_passwordPostSalt = passwordPostSalt;
}

bool CredentialPassword::checkPassword(const Authentication::User &user, const CStringHash &authinfo)
{
    QString password = authinfo.value(m_passwordField);
    QString storedPassword = user.value(m_passwordField);

    if (m_passwordType == None) {
        return true;
    } else if (m_passwordType == Clear) {
        return storedPassword == password;
    } else if (m_passwordType == Hashed) {
        QCryptographicHash hash(m_hashType);
        hash.addData(m_passwordPreSalt.toUtf8());
        hash.addData(password.toUtf8());
        hash.addData(m_passwordPostSalt.toUtf8());
        QByteArray result =  hash.result();

        return storedPassword == result ||
                storedPassword == result.toHex() ||
                storedPassword == result.toBase64();
    } else if (m_passwordType == SelfCheck) {
        return user.checkPassword(password);
    }

    return false;
}
