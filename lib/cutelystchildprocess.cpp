#include "cutelystchildprocess_p.h"

#include "cutelystenginehttp.h"
#include "cutelystcontroller.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <QDebug>

#include <QMetaType>
#include <QMetaClassInfo>

CutelystChildProcess::CutelystChildProcess(bool &childProcess, QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystChildProcessPrivate(this))
{
    Q_D(CutelystChildProcess);

    childProcess = false;
    int sv[2];

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        exit(1);
    }

    switch ((d->childPID = fork())) {
    case 0:
        childProcess = true;
        close(sv[0]);
        initChild(sv[1]);
        break;
    case -1:
        perror("fork");
        return;
    default:
        close(sv[1]);
        d->parentFD = sv[0];
        break;
    }
}

CutelystChildProcess::~CutelystChildProcess()
{
    Q_D(CutelystChildProcess);
    if (d->childPID > 0) {
        // Finish the child process
        kill(d->childPID, SIGTERM);

        bool died = false;
        for (int i = 0; !died && i < 5; ++i) {
            int status;
            sleep(1);
            if (waitpid(d->childPID, &status, WNOHANG) == d->childPID) {
                died = true;
            }
        }

        if (!died) {
            qWarning() << "Forcing the Kill of child:" << d->childPID;
            kill(d->childPID, SIGKILL);
        }
    }
    delete d_ptr;
}

bool CutelystChildProcess::initted() const
{
    Q_D(const CutelystChildProcess);
    return d->error.isEmpty() && (d->childFD || d->parentFD);
}

bool CutelystChildProcess::sendFD(int fd)
{
    Q_D(CutelystChildProcess);
    char buf[2];
    buf[0] = 1;
    buf[1] = '\0';
    sprintf(buf, "%d", fd);
    return d->sendFD(d->parentFD, buf, 1, fd) == 1;
}

void CutelystChildProcess::initChild(int socket)
{
    QSocketNotifier *notifier = new QSocketNotifier(socket, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated,
            this, &CutelystChildProcess::gotFD);

    qRegisterMetaType<CutelystController*>();
//    QMetaType meta;

    int metaType = QMetaType::User;
    while (QMetaType::isRegistered(metaType)) {
        const QMetaObject *meta = QMetaType::metaObjectForType(metaType);
        const QMetaObject *superMeta = meta->superClass();
        qDebug() << Q_FUNC_INFO << superMeta->className() << meta->classInfoCount();
        if (qstrcmp(superMeta->className(), "CutelystController") == 0) {
            // App controller
            qDebug() << Q_FUNC_INFO << "Found a controller:" << meta->className();
            int i = 0;
            while (i < meta->methodCount()) {
                qDebug() << meta->method(i).name() << meta->method(i).attributes();
                qDebug() << meta->method(i).parameterTypes();
                ++i;
            }
        }

        if (meta->classInfoCount()) {
            QMetaClassInfo classInfo = meta->classInfo(0);
            qDebug() << Q_FUNC_INFO << classInfo.name() << classInfo.value();
        }
        ++metaType;
    }
}

void CutelystChildProcess::gotFD(int socket)
{
    Q_D(CutelystChildProcess);

    int fd;
    char buf[16];
    if (d->readFD(socket, buf, sizeof(buf), &fd) == 1) {
        CutelystEngineHttp *engine = new CutelystEngineHttp(fd, this);
        if (engine->isValid()) {

        } else {
            delete engine;
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Failed to read file descriptor";
    }
}

CutelystChildProcessPrivate::CutelystChildProcessPrivate(CutelystChildProcess *parent) :
    q_ptr(parent),
    notifier(0),
    childFD(0),
    parentFD(0),
    childPID(-1)
{
}

CutelystChildProcessPrivate::~CutelystChildProcessPrivate()
{
    delete notifier;
}

ssize_t CutelystChildProcessPrivate::sendFD(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr cmsghdr;
        char control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

//        qDebug("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
//        qDebug("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0) {
        perror ("sendmsg");
    }

    return size;
}

ssize_t CutelystChildProcessPrivate::readFD(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t size;

    if (fd) {
        struct msghdr   msg;
        struct iovec    iov;
        union {
            struct cmsghdr  cmsghdr;
            char        control[CMSG_SPACE(sizeof (int))];
        } cmsgu;
        struct cmsghdr  *cmsg;

        iov.iov_base = buf;
        iov.iov_len = bufsize;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);
        if (size < 0) {
            qWarning() << Q_FUNC_INFO << "Failed to receive FD:" << size;
            perror ("recvmsg");
            exit(1);
        }

        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n",
                         cmsg->cmsg_level);
                exit(1);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n",
                         cmsg->cmsg_type);
                exit(1);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
//            qDebug("received fd %d\n", *fd);
        } else {
            *fd = -1;
        }
    } else {
        size = read(sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }

    return size;
}
