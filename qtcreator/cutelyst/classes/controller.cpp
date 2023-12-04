%{Cpp:LicenseTemplate}\
#include "%{HdrFileName}"

using namespace Cutelyst;
%{JS: Cpp.openNamespaces('%{Class}')}\

%{CN}::%{CN}(QObject *parent)
    : Controller(parent)
{
}

void %{CN}::index(Context *c)
{
    // FIXME: Implement me!
}
@if %{BeginMethod}

bool %{CN}::Begin(Context *c)
{
    // FIXME: Implement me!
    return true;
}
@endif
@if %{AutoMethod}

bool %{CN}::Auto(Context *c)
{
    // FIXME: Implement me!
    return true;
}
@endif
@if %{EndMethod}

bool %{CN}::End(Context *c)
{
    // FIXME: Implement me!
    return true;
}
@endif

%{JS: Cpp.closeNamespaces('%{Class}')}\
