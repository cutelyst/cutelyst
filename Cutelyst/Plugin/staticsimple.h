#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <QObject>

#include "plugin.h"

namespace Cutelyst {

class Context;
class Application;

namespace Plugin {

class StaticSimple : public AbstractPlugin
{
    Q_OBJECT
public:
    explicit StaticSimple(const QString &path = QString(), QObject *parent = 0);

    void setRootDir(const QString &path);

    bool setup(Context *ctx);

private:
    void beforePrepareAction(bool *skipMethod);
    bool locateStaticFile(Context *ctx, QString &path);

    QString m_rootDir;
};

}

}

#endif // CPSTATICSIMPLE_H
