#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <Cutelyst/plugin.h>

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

    bool setup(Context *ctx);

    virtual bool isApplicationPlugin() const;

protected:
    StaticSimplePrivate *d_ptr;

private:
    void beforePrepareAction(bool *skipMethod);
    bool locateStaticFile(Context *ctx, const QString &relPath);
};

}

#endif // CPSTATICSIMPLE_H
