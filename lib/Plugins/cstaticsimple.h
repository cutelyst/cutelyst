#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <QObject>
#include "cutelystplugin.h"

class Cutelyst;
class CutelystApplication;
class CPStaticSimple : public CutelystPlugin
{
    Q_OBJECT
public:
    explicit CPStaticSimple(QObject *parent = 0);

    bool setup(CutelystApplication *app);

private:
    void beforePrepareAction(Cutelyst *c, bool *skipMethod);
};

#endif // CPSTATICSIMPLE_H
