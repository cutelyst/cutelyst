#ifndef CUTELYSTPLUGIN_H
#define CUTELYSTPLUGIN_H

#include <QObject>

class CutelystApplication;
class CutelystPlugin : public QObject
{
    Q_OBJECT
public:
    explicit CutelystPlugin(QObject *parent = 0);

    /**
     * Reimplement this if you need to connect to
     * the signals emitted from CutelystApplication
     */
    virtual bool setup(CutelystApplication *app);
};

#endif // CUTELYSTPLUGIN_H
