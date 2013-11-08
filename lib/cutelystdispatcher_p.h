#ifndef CUTELYSTDISPATCHER_P_H
#define CUTELYSTDISPATCHER_P_H

#include "cutelystdispatcher.h"

class CutelystDispatcherPrivate
{
public:
    CutelystActionList getContainers(const QString &ns);
    QHash<QString, CutelystAction*> actionHash;
    QHash<QString, CutelystActionList> containerHash;
    QList<CutelystDispatchType*> dispatchers;
};

#endif // CUTELYSTDISPATCHER_P_H
