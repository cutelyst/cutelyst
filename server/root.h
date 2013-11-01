#ifndef ROOT_H
#define ROOT_H

#include <cutelystcontroller.h>

class Root : public CutelystController
{
    Q_OBJECT
public:
    Q_INVOKABLE Root();

private:
    Q_CLASSINFO("begin_Path", "/home")
    Q_INVOKABLE void begin(CutelystContext *c, const QString &nome);
    void begin(CutelystContext *c, const QString &nome, const QString &idade);
};

#endif // ROOT_H
