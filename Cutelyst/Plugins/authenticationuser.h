#ifndef AUTHENTICATIONUSER_H
#define AUTHENTICATIONUSER_H

#include <Cutelyst/plugin.h>

namespace Cutelyst {

class AuthenticationRealm;
class AuthenticationUser : public CStringHash
{
public:
    AuthenticationUser();
    AuthenticationUser(const QString &id);
    virtual ~AuthenticationUser();

    /**
     * A unique ID by which a AuthenticationUser can be retrieved from the store.
     */
    QString id() const;
    void setId(const QString &id);
    bool isNull() const;

    AuthenticationRealm *authRealm();
    void setAuthRealm(AuthenticationRealm *authRealm);

    virtual bool checkPassword(const QString &password) const;

private:
    QString m_id;
    AuthenticationRealm *m_realm;
};

}

Q_DECLARE_METATYPE(Cutelyst::AuthenticationUser)
QDataStream &operator<<(QDataStream &out, const Cutelyst::AuthenticationUser &myObj);
QDataStream &operator>>(QDataStream &in, Cutelyst::AuthenticationUser &myObj);

#endif // AUTHENTICATIONUSER_H
