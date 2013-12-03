#ifndef CLEARSILVER_P_H
#define CLEARSILVER_P_H

#include "clearsilver.h"

#include <ClearSilver/ClearSilver.h>

class ClearSilverPrivate
{
public:
    HDF *hdfForStash(const QVariantHash &stash);

    QString rootPath;
};

#endif // CLEARSILVER_P_H
