#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <QObject>

#include "plugin.h"

namespace Cutelyst {

class Context;
class CutelystApplication;

namespace CutelystPlugin {

class StaticSimple : public Plugin
{
    Q_OBJECT
public:
    explicit StaticSimple(const QString &path = QString(), QObject *parent = 0);

    void setRootDir(const QString &path);

    bool setup(CutelystApplication *app);

private:
    void beforePrepareAction(Context *ctx, bool *skipMethod);
    bool locateStaticFile(Context *ctx, QString &path);

    QString m_rootDir;
};

}

}

#endif // CPSTATICSIMPLE_H
