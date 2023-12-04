%{Cpp:LicenseTemplate}\
#ifndef %{GUARD}
#define %{GUARD}

#include <Cutelyst/Controller>

%{JS: Cpp.openNamespaces('%{Class}')}\
using namespace Cutelyst;

class %{CN} : public Controller
{
     Q_OBJECT
@if %{CustomNamespace}
     C_NAMESPACE("%{CustomNamespaceValue}")
@endif
public:
    explicit %{CN}(QObject *parent = nullptr);

    C_ATTR(index, :Path)
    void index(Context *c);
    
private Q_SLOTS:
@if %{BeginMethod}
    bool Begin(Context *c);

@endif
@if %{AutoMethod}
    bool Auto(Context *c);

@endif
@if %{EndMethod}
    bool End(Context *c);

@endif
};
%{JS: Cpp.closeNamespaces('%{Class}')}
#endif // %{GUARD}\
