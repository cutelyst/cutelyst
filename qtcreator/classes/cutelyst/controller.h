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
    explicit %{CN}(QObject *parent = 0);

    C_ATTR(index, :Path)
    void index(Context *c);
    
private Q_SLOTS:
@if %{BeginMethod}
    bool Begin(Context *c) override;

@endif
@if %{AutoMethod}
    bool Auto(Context *c) override;

@endif
@if %{EndMethod}
    bool End(Context *c) override;

@endif
};
%{JS: Cpp.closeNamespaces('%{Class}')}
#endif // %{GUARD}\
