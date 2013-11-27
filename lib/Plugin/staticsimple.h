#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <QObject>

#include "plugin.h"

class Cutelyst;
class CutelystApplication;

namespace CutelystPlugin {

class StaticSimple : public Plugin
{
    Q_OBJECT
public:
    explicit StaticSimple(QObject *parent = 0);

    void setRootDir(const QString &path);

    bool setup(CutelystApplication *app);

private:
    void beforePrepareAction(Cutelyst *c, bool *skipMethod);
    bool locateStaticFile(Cutelyst *c, QString &path);

    QString m_rootDir;
};

}

#endif // CPSTATICSIMPLE_H
