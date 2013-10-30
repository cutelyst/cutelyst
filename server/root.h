#ifndef ROOT_H
#define ROOT_H

#include <cutelystcontroller.h>

class Root : public CutelystController
{
    Q_OBJECT
public:
    Root();

public slots:
    void begin(Cutelyst *c, const QString &nome, QString foo, QString &bar, QString *teste);
};

#endif // ROOT_H
