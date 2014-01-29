#ifndef GRANTLEE_VIEW_P_H
#define GRANTLEE_VIEW_P_H

#include "grantleeview.h"

#include <grantlee/engine.h>

namespace Cutelyst {

class GrantleeViewPrivate
{
public:
    QString includePath;
    QString extension;
    QString wrapper;
    Grantlee::Engine *engine;
};

}

#endif // GRANTLEE_VIEW_P_H
