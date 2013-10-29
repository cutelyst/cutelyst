#ifndef CUTELYSTAPPLICATION_P_H
#define CUTELYSTAPPLICATION_P_H

#include "cutelystapplication.h"

#include <QSocketNotifier>
#include <QTcpServer>

class CutelystEngine;
class CutelystApplicationPrivate
{
    Q_DECLARE_PUBLIC(CutelystApplication)
public:
    CutelystApplicationPrivate(CutelystApplication *parent);
    ~CutelystApplicationPrivate();

    CutelystApplication *q_ptr;
    QSocketNotifier *notifier;
    QString error;
    int childFD;
    int parentFD;
    int childPID;

    quint16 port;
    QHostAddress address;
    QString pluginApplication;
    QTcpServer *server;
    QList<CutelystEngine*> child;
};

#endif // CUTELYSTAPPLICATION_P_H
