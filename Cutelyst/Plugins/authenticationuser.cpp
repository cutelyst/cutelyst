#include "authenticationuser.h"

using namespace Cutelyst;

AuthenticationUser::AuthenticationUser()
{

}

AuthenticationUser::AuthenticationUser(const QString &id) :
    m_id(id)
{

}

AuthenticationUser::~AuthenticationUser()
{
}

QString AuthenticationUser::id() const
{
    return m_id;
}

void AuthenticationUser::setId(const QString &id)
{
    m_id = id;
}

bool AuthenticationUser::isNull() const
{
    return m_id.isNull();
}

AuthenticationRealm *AuthenticationUser::authRealm()
{
    return m_realm;
}

void AuthenticationUser::setAuthRealm(AuthenticationRealm *authRealm)
{
    m_realm = authRealm;
}

bool AuthenticationUser::checkPassword(const QString &password) const
{
    Q_UNUSED(password)
    return false;
}

QDataStream &operator<<(QDataStream &out, const AuthenticationUser &user)
{
    out << user.id() << static_cast<CStringHash>(user);
    return out;
}

QDataStream &operator>>(QDataStream &in, AuthenticationUser &user)
{
    QString id;
    CStringHash hash;
    in >> id >> hash;
    user.setId(id);
    user.swap(hash);
    return in;
}
