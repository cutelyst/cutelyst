#include "cutelystplugin.h"

CutelystPlugin::CutelystPlugin(QObject *parent) :
    QObject(parent)
{
}

bool CutelystPlugin::setup(CutelystApplication *app)
{
    Q_UNUSED(app)
    return true;
}
