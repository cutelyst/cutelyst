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
    explicit StaticSimple(const QString &path = QString(), QObject *parent = 0);
    virtual ~StaticSimple();

    void setRootDir(const QString &path);

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
