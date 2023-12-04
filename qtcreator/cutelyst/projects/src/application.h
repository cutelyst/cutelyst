%{Cpp:LicenseTemplate}\
#ifndef %{GUARD}
#define %{GUARD}

#include <Cutelyst/Application>

using namespace Cutelyst;

class %{ProjectName} : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "%{ProjectName}")
public:
    Q_INVOKABLE explicit %{ProjectName}(QObject *parent = nullptr);
    ~%{ProjectName}();

    bool init() override;
};

#endif // %{GUARD}\

