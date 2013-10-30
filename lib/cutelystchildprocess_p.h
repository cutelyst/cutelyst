#ifndef CUTELYSTCHILDPROCESS_P_H
#define CUTELYSTCHILDPROCESS_P_H

#include "cutelystchildprocess.h"

#include <QSocketNotifier>

class CutelystChildProcessPrivate
{
    Q_DECLARE_PUBLIC(CutelystChildProcess)
public:
    CutelystChildProcessPrivate(CutelystChildProcess *parent);
    ~CutelystChildProcessPrivate();

    void gotFD(int socket);
    ssize_t sendFD(int sock, void *buf, ssize_t buflen, int fd);
    ssize_t readFD(int sock, void *buf, ssize_t bufsize, int *fd);

    CutelystChildProcess *q_ptr;
    QSocketNotifier *notifier;
    QString error;
    int childFD;
    int parentFD;
    int childPID;
};

#endif // CUTELYSTCHILDPROCESS_P_H
