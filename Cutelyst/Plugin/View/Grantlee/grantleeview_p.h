#ifndef GRANTLEE_VIEW_P_H
#define GRANTLEE_VIEW_P_H

#include "grantleeview.h"

#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

namespace Cutelyst {

class GrantleeViewPrivate
{
public:
    QString includePath;
    QString extension;
    QString wrapper;
    Grantlee::Engine *engine;
    Grantlee::FileSystemTemplateLoader::Ptr loader;
};

}

#endif // GRANTLEE_VIEW_P_H
