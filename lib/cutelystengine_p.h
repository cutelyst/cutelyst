#ifndef CUTELYSTENGINE_P_H
#define CUTELYSTENGINE_P_H

#include "cutelystengine.h"

#include <QTcpSocket>

class CutelystEnginePrivate
{
    Q_DECLARE_PUBLIC(CutelystEngine)
public:
    CutelystEnginePrivate(CutelystEngine *parent);
    ~CutelystEnginePrivate();

    CutelystEngine *q_ptr;
    QTcpSocket *socket;
    bool valid;
    CutelystRequest *request;
};

#endif // CUTELYSTENGINE_P_H
