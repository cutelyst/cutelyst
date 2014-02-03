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
    StaticSimple(Application *parent, const QString &path = QString());

    void setRootDir(const QString &path);

    bool setup(Context *ctx);

    virtual bool isApplicationPlugin() const;

private:
    void beforePrepareAction(bool *skipMethod);
    bool locateStaticFile(Context *ctx, QString &path);

    QString m_rootDir;
};

}

}

#endif // CPSTATICSIMPLE_H
