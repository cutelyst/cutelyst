#ifndef CUTELYSTDISPATCHER_P_H
#define CUTELYSTDISPATCHER_P_H

#include "cutelystdispatcher.h"

class CutelystDispatcherPrivate
{
public:
    QHash<QString, CutelystAction*> actions;
    QList<CutelystDispatchType*> dispatchers;
};

#endif // CUTELYSTDISPATCHER_P_H
