#include "%{AppHdrFileName}"

#include <Cutelyst/Plugins/StaticSimple/staticsimple.h>

#include "root.h"

using namespace Cutelyst;

%{ProjectName}::%{ProjectName}(QObject *parent) : Application(parent)
{
}

%{ProjectName}::~%{ProjectName}()
{
}

bool %{ProjectName}::init()
{
    new Root(this);

    new StaticSimple(this);

    return true;
}

