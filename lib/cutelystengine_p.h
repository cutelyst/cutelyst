#ifndef CUTELYSTENGINE_P_H
#define CUTELYSTENGINE_P_H

#include "cutelystengine.h"

#include <QSocketNotifier>

class CutelystEnginePrivate
{
    Q_DECLARE_PUBLIC(CutelystEngine)
public:
    CutelystEnginePrivate(CutelystEngine *parent);
    ~CutelystEnginePrivate();

    void gotFD(int socket);
    ssize_t sendFD(int sock, void *buf, ssize_t buflen, int fd);
    ssize_t readFD(int sock, void *buf, ssize_t bufsize, int *fd);

    CutelystEngine *q_ptr;
    QSocketNotifier *notifier;
    QString error;
    int childFD;
    int parentFD;
    int childPID;
};

#endif // CUTELYSTENGINE_P_H
