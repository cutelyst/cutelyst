#ifndef USERS_H
#define USERS_H

#include <cutelystcontroller.h>

class Users : public CutelystController
{
    Q_OBJECT
public:
    Q_INVOKABLE Users();

    Q_INVOKABLE void list(const QString &name, const QString &age, Args, Local);

    Q_INVOKABLE void Begin();
    Q_INVOKABLE void Auto();
//    Q_INVOKABLE void End();
};

#endif // USERS_H
