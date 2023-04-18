/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "systemdnotify.h"

#include "server.h"

#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QScopeGuard>
#include <QTimer>

/* The first passed file descriptor is fd 3 */
#define SD_LISTEN_FDS_START 3

Q_LOGGING_CATEGORY(C_SYSTEMD, "cutelyst.systemd", QtWarningMsg)

using namespace Cutelyst;

namespace Cutelyst {

class systemdNotifyPrivate
{
public:
    struct msghdr *notification_object = nullptr;
    QTimer *watchdog                   = nullptr;
    int notification_fd                = 0;
    int watchdog_usec                  = 0;
};

} // namespace Cutelyst

systemdNotify::systemdNotify(QObject *parent)
    : QObject(parent)
    , d_ptr(new systemdNotifyPrivate)
{
    Q_D(systemdNotify);

    struct sockaddr_un *sd_sun;
    struct msghdr *msghdr;

    d->notification_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (d->notification_fd < 0) {
        qCWarning(C_SYSTEMD, "socket()");
        return;
    }

    auto systemd_socket = getenv("NOTIFY_SOCKET");
    size_t len          = strlen(systemd_socket);
    sd_sun              = new struct sockaddr_un;
    memset(sd_sun, 0, sizeof(struct sockaddr_un));
    sd_sun->sun_family = AF_UNIX;
    strncpy(sd_sun->sun_path, systemd_socket, qMin(len, sizeof(sd_sun->sun_path)));
    if (sd_sun->sun_path[0] == '@')
        sd_sun->sun_path[0] = 0;

    msghdr = new struct msghdr;
    memset(msghdr, 0, sizeof(struct msghdr));

    msghdr->msg_iov = new struct iovec[3];
    memset(msghdr->msg_iov, 0, sizeof(struct iovec) * 3);

    msghdr->msg_name    = sd_sun;
    msghdr->msg_namelen = sizeof(struct sockaddr_un) - (sizeof(sd_sun->sun_path) - len);

    d->notification_object = msghdr;
}

systemdNotify::~systemdNotify()
{
    Q_D(systemdNotify);
    if (d->notification_object) {
        delete static_cast<struct sockaddr_un *>(d->notification_object->msg_name);
        delete[] d->notification_object->msg_iov;
        delete d->notification_object;
    }
    delete d_ptr;
}

int systemdNotify::watchdogUSec() const
{
    Q_D(const systemdNotify);
    return d->watchdog_usec;
}

bool systemdNotify::setWatchdog(bool enable, int usec)
{
    Q_D(systemdNotify);
    if (enable) {
        d->watchdog_usec = usec;
        if (d->watchdog_usec > 0) {
            if (!d->watchdog) {
                // Issue first ping immediately
                d->watchdog = new QTimer(this);
                // SD recommends half the defined interval
                d->watchdog->setInterval(d->watchdog_usec / 1000 / 2);
                sendWatchdog(QByteArrayLiteral("1"));
                connect(d->watchdog, &QTimer::timeout, this, [this] {
                    sendWatchdog(QByteArrayLiteral("1"));
                });
                d->watchdog->start();
                qCInfo(C_SYSTEMD) << "watchdog enabled" << d->watchdog_usec << d->watchdog->interval();
            }
            return true;
        } else {
            return false;
        }
    } else {
        delete d->watchdog;
        d->watchdog = nullptr;
    }
    return true;
}

void systemdNotify::sendStatus(const QByteArray &data)
{
    Q_D(systemdNotify);
    Q_ASSERT(d->notification_fd);

    struct msghdr *msghdr = d->notification_object;
    struct iovec *iovec   = msghdr->msg_iov;

    iovec[0].iov_base = const_cast<char *>("STATUS=");
    iovec[0].iov_len  = 7;

    iovec[1].iov_base = const_cast<char *>(data.constData());
    iovec[1].iov_len  = data.size();

    iovec[2].iov_base = const_cast<char *>("\n");
    iovec[2].iov_len  = 1;

    msghdr->msg_iovlen = 3;

    if (sendmsg(d->notification_fd, msghdr, 0) < 0) {
        qCWarning(C_SYSTEMD, "sendStatus()");
    }
}

void systemdNotify::sendWatchdog(const QByteArray &data)
{
    Q_D(systemdNotify);
    Q_ASSERT(d->notification_fd);

    struct msghdr *msghdr = d->notification_object;
    struct iovec *iovec   = msghdr->msg_iov;

    iovec[0].iov_base = const_cast<char *>("WATCHDOG=");
    iovec[0].iov_len  = 9;

    iovec[1].iov_base = const_cast<char *>(data.constData());
    iovec[1].iov_len  = data.size();

    iovec[2].iov_base = const_cast<char *>("\n");
    iovec[2].iov_len  = 1;

    msghdr->msg_iovlen = 3;

    if (sendmsg(d->notification_fd, msghdr, 0) < 0) {
        qCWarning(C_SYSTEMD, "sendWatchdog()");
    }
}

void systemdNotify::sendReady(const QByteArray &data)
{
    Q_D(systemdNotify);
    Q_ASSERT(d->notification_fd);

    struct msghdr *msghdr = d->notification_object;
    struct iovec *iovec   = msghdr->msg_iov;

    iovec[0].iov_base = const_cast<char *>("READY=");
    iovec[0].iov_len  = 6;

    iovec[1].iov_base = const_cast<char *>(data.constData());
    iovec[1].iov_len  = data.size();

    iovec[2].iov_base = const_cast<char *>("\n");
    iovec[2].iov_len  = 1;

    msghdr->msg_iovlen = 3;

    if (sendmsg(d->notification_fd, msghdr, 0) < 0) {
        qCWarning(C_SYSTEMD, "sendReady()");
    }
}

int systemdNotify::sd_watchdog_enabled(bool unset)
{
    int ret;
    auto cleanup = qScopeGuard([unset, &ret] {
        if (unset && ret > 0) {
            qunsetenv("WATCHDOG_USEC");
            qunsetenv("WATCHDOG_PID");
        }
    });

    QByteArray wusec = qgetenv("WATCHDOG_USEC");
    bool ok;
    ret = wusec.toInt(&ok);
    if (!ok) {
        return -1;
    }

    if (qEnvironmentVariableIsSet("WATCHDOG_PID")) {
        QByteArray wpid = qgetenv("WATCHDOG_PID");
        qint64 pid      = wpid.toLongLong(&ok);
        if (pid != qApp->applicationPid()) {
            return -2;
        }
    }

    return ret;
}

bool systemdNotify::is_systemd_notify_available()
{
    return qEnvironmentVariableIsSet("NOTIFY_SOCKET");
}

int fd_cloexec(int fd, bool cloexec)
{
    int flags, nflags;

    Q_ASSERT(fd >= 0);

    flags = fcntl(fd, F_GETFD, 0);
    if (flags < 0)
        return -errno;

    if (cloexec)
        nflags = flags | FD_CLOEXEC;
    else
        nflags = flags & ~FD_CLOEXEC;

    if (nflags == flags)
        return 0;

    if (fcntl(fd, F_SETFD, nflags) < 0)
        return -errno;

    return 0;
}

int sd_listen_fds()
{
    const QByteArray listenPid = qgetenv("LISTEN_PID");
    bool ok;
    qint64 pid = static_cast<pid_t>(listenPid.toLongLong(&ok));
    if (!ok) {
        return 0;
    }

    /* Is this for us? */
    if (QCoreApplication::applicationPid() != pid) {
        return 0;
    }

    const QByteArray listenFDS = qgetenv("LISTEN_FDS");
    int n                      = listenFDS.toInt(&ok);
    if (!ok) {
        return 0;
    }

    Q_ASSERT(SD_LISTEN_FDS_START < INT_MAX);
    if (n <= 0 || n > INT_MAX - SD_LISTEN_FDS_START) {
        return -EINVAL;
    }

    qCInfo(C_SYSTEMD, "systemd socket activation detected");

    int r = 0;
    for (int fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd++) {
        r = fd_cloexec(fd, true);
        if (r < 0) {
            return r;
        }
    }

    r = n;

    return r;
}

std::vector<int> systemdNotify::listenFds(bool unsetEnvironment)
{
    std::vector<int> ret;
    int maxFD;
    if ((maxFD = sd_listen_fds()) > 0) {
        for (int fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + maxFD; ++fd) {
            ret.push_back(fd);
        }
    }

    if (unsetEnvironment) {
        qunsetenv("LISTEN_PID");
        qunsetenv("LISTEN_FDS");
        qunsetenv("LISTEN_FDNAMES");
    }

    return ret;
}

#include "moc_systemdnotify.cpp"
