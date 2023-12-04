%{Cpp:LicenseTemplate}\
#include "%{AppHdrFileName}"

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

    return true;
}

