#ifndef ROOT_H
#define ROOT_H

#include <cutelystcontroller.h>

class Root : public CutelystController
{
    Q_OBJECT
    Q_CLASSINFO("Namespace", "")
public:
    Q_INVOKABLE Root();

public:
    Q_INVOKABLE void hugeNameQuiteLong(Cutelyst *c, const QString &nome, Local);
    Q_CLASSINFO("begin_Path", "/home")
    Q_CLASSINFO("begin_Chained", "/")
    Q_CLASSINFO("begin_Path", "/")
    Q_INVOKABLE void begin(Cutelyst *c, const QString &name, Path);
    Q_INVOKABLE void users(Cutelyst *c, const QString &name, const QString &age, Args, Local);
    Q_INVOKABLE void admin(Cutelyst *c, const QString &name, const QString &age, Global);

    Q_INVOKABLE void Begin(Cutelyst *c);
    Q_INVOKABLE void Auto(Cutelyst *c);
    Q_INVOKABLE void End(Cutelyst *c);
};

#endif // ROOT_H
