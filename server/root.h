#ifndef ROOT_H
#define ROOT_H

#include <cutelystcontroller.h>

class Root : public CutelystController
{
    Q_OBJECT
    Q_CLASSINFO("Namespace", "")
public:
    Q_INVOKABLE Root();

private:
    Q_CLASSINFO("begin_Path", "/home")
    Q_INVOKABLE void begin(const QString &nome);
    Q_INVOKABLE void begin(const QString &nome, const QString &idade, CaptureArgs);
};

#endif // ROOT_H
