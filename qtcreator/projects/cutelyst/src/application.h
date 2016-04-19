%{Cpp:LicenseTemplate}\
#ifndef %{GUARD}
#define %{GUARD}

#include <Cutelyst/Application>

using namespace Cutelyst;

class %{ProjectName} : public Application
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit %{ProjectName}(QObject *parent = 0);
    ~%{ProjectName}();

    bool init();
};

#endif // %{GUARD}\

