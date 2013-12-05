#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/controller.h>

using namespace Cutelyst;

class Root : public Controller
{
    Q_OBJECT
    Q_CLASSINFO("Namespace", "")
public:
    Q_INVOKABLE Root();

public:
    Q_INVOKABLE void hugeNameQuiteLong(Context *ctx, const QString &nome, Local);
    Q_CLASSINFO("begin_Path", "/home")
    Q_CLASSINFO("begin_Chained", "/")
    Q_CLASSINFO("begin_Path", "/")
    Q_INVOKABLE void begin(Context *ctx, const QString &name, Path);
    Q_INVOKABLE void users(Context *ctx, const QString &name, const QString &age, Args, Local);
    Q_INVOKABLE void admin(Context *ctx, const QString &name, const QString &age, Global);

    Q_INVOKABLE void Begin(Context *ctx);
    Q_INVOKABLE bool Auto(Context *ctx);
    Q_INVOKABLE void End(Context *ctx);
};

#endif // ROOT_H
