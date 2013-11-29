#ifndef USERS_H
#define USERS_H

#include <cutelystcontroller.h>

class Users : public CutelystController
{
    Q_OBJECT
public:
    Q_INVOKABLE Users();

    Q_INVOKABLE void list(Cutelyst *c, const QString &name, const QString &age, Args, Local);

    Q_INVOKABLE void Begin(Cutelyst *c);
    Q_INVOKABLE bool Auto(Cutelyst *c);
//    Q_INVOKABLE void End();
};

#endif // USERS_H
