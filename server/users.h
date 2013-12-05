#ifndef USERS_H
#define USERS_H

#include <cutelystcontroller.h>

using namespace Cutelyst;

class Users : public CutelystController
{
    Q_OBJECT
public:
    Q_INVOKABLE Users();

    Q_INVOKABLE void list(Context *ctx, const QString &name, const QString &age, Args, Local);

    Q_INVOKABLE void Begin(Context *ctx);
    Q_INVOKABLE bool Auto(Context *ctx);
//    Q_INVOKABLE void End();
};

#endif // USERS_H
