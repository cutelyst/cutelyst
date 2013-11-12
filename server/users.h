#ifndef USERS_H
#define USERS_H

#include <cutelystcontroller.h>

class Users : public CutelystController
{
    Q_OBJECT
public:
    Q_INVOKABLE Users();

    Q_INVOKABLE void list(const QString &name, const QString &age, Args, Local);
};

#endif // USERS_H
