#include "childprocess.h"

#include "cutelysthttprequest.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <QDebug>

ChildProcess::ChildProcess(bool &childProcess, QObject *parent) :
    QObject(parent),
    m_childFD(0),
    m_parentFD(0),
    m_childPID(-1)
{
    childProcess = false;
    int sv[2];

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        exit(1);
    }
    switch ((m_childPID = fork())) {
    case 0:
        childProcess = true;
        close(sv[0]);
        initChild(sv[1]);
        break;
    case -1:
        perror("fork");
        exit(1);
    default:
        close(sv[1]);
        m_parentFD = sv[0];
        break;
    }
}

ChildProcess::~ChildProcess()
{
    if (m_childPID > 0) {
        // Finish the child process
        kill(m_childPID, SIGTERM);

        bool died = false;
        for (int i = 0; !died && i < 5; ++i) {
            int status;
            sleep(1);
            if (waitpid(m_childPID, &status, WNOHANG) == m_childPID) {
                died = true;
            }
        }

        if (!died) {
            qWarning() << "Forcing the Kill of child:" << m_childPID;
            kill(m_childPID, SIGKILL);
        }
    }
}

bool ChildProcess::initted() const
{
    return m_error.isEmpty() && (m_childFD || m_parentFD);
}

bool ChildProcess::sendFD(int fd)
{
    char buf[2];
    buf[0] = 1;
    buf[1] = '\0';
    sprintf(buf, "%d", fd);
    return sendFD(m_parentFD, buf, 1, fd) == 1;
}

void ChildProcess::initChild(int socket)
{
    QSocketNotifier *notifier = new QSocketNotifier(socket, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated,
            this, &ChildProcess::getFD);
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
}

void ChildProcess::getFD(int socket)
{
    qDebug() << Q_FUNC_INFO << socket;

    char buf[16];

    int fd;
    if (readFD(socket, buf, sizeof(buf), &fd) == 1) {
        qDebug() << "GOT FD" << fd;
        QTcpSocket *socket = new QTcpSocket(this);
        if (socket->setSocketDescriptor(fd)) {
            CutelystHttpRequest *request = new CutelystHttpRequest(socket, this);
        } else {
//            socket->close();
//            delete socket;
        }
    } else {
        qDebug() << "FAILED to get FD";
    }
}

ssize_t ChildProcess::sendFD(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
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

        qDebug("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        qDebug("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);

    if (size < 0)
        perror ("sendmsg");
    return size;
}

ssize_t ChildProcess::readFD(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t     size;

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
            qDebug() << Q_FUNC_INFO << "failed" << size;
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
            printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
        size = read (sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }
    return size;
}
