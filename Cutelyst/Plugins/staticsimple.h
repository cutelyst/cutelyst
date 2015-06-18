#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <Cutelyst/plugin.h>
#include <Cutelyst/context.h>

namespace Cutelyst {

class StaticSimplePrivate;
class StaticSimple : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StaticSimple)
public:
    StaticSimple(Application *parent);
    virtual ~StaticSimple();

    void setIncludePaths(const QStringList &paths);

    void setDirs(const QStringList &dirs);

    virtual bool setup(Application *app);

protected:
    StaticSimplePrivate *d_ptr;

private:
    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateStaticFile(Context *c, const QString &relPath);
};

}

#endif // CPSTATICSIMPLE_H
