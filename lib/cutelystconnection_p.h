#ifndef CUTELYSTCONNECTION_P_H
#define CUTELYSTCONNECTION_P_H

#include "cutelystconnection.h"

#include <QTcpSocket>

class CutelystConnectionPrivate
{
    Q_DECLARE_PUBLIC(CutelystConnection)
public:
    CutelystConnectionPrivate(CutelystConnection *parent);
    ~CutelystConnectionPrivate();

    CutelystConnection *q_ptr;
    QTcpSocket *socket;
    bool valid;
    CutelystRequest *request;
};

#endif // CUTELYSTCONNECTION_P_H
