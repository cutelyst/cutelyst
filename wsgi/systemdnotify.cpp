/*
 * Copyright (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "systemdnotify.h"

#include "wsgi.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>


#include <unistd.h>
#include <fcntl.h>

#include <QCoreApplication>
#include <QLoggingCategory>

/* The first passed file descriptor is fd 3 */
#define SD_LISTEN_FDS_START 3

Q_LOGGING_CATEGORY(WSGI_SYSTEMD, "wsgi.systemd", QtWarningMsg)

using namespace CWSGI;

namespace CWSGI {

class systemdNotifyPrivate
{
public:
    struct msghdr *notification_object = nullptr;
    int notification_fd = 0;
};

}

systemdNotify::systemdNotify(const char *systemd_socket, QObject *parent) : QObject(parent)
  , d_ptr(new systemdNotifyPrivate)
{
    Q_D(systemdNotify);

    struct sockaddr_un *sd_sun;
    struct msghdr *msghdr;

    d->notification_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (d->notification_fd < 0) {
        qCWarning(WSGI_SYSTEMD, "socket()");
        return;
    }

    size_t len = strlen(systemd_socket);
    sd_sun = new struct sockaddr_un;
    memset(sd_sun, 0, sizeof(struct sockaddr_un));
    sd_sun->sun_family = AF_UNIX;
    strncpy(sd_sun->sun_path, systemd_socket, qMin(len, sizeof(sd_sun->sun_path)));
    if (sd_sun->sun_path[0] == '@')
        sd_sun->sun_path[0] = 0;

    msghdr = new struct msghdr;
    memset(msghdr, 0, sizeof(struct msghdr));

    msghdr->msg_iov = new struct iovec[3];
    memset(msghdr->msg_iov, 0, sizeof(struct iovec) * 3);

    msghdr->msg_name = sd_sun;
    msghdr->msg_namelen = sizeof(struct sockaddr_un) - (sizeof(sd_sun->sun_path) - len);

    d->notification_object = msghdr;
}

systemdNotify::~systemdNotify()
{
    Q_D(systemdNotify);
    if (d->notification_object) {
        delete static_cast<struct sockaddr_un *>(d->notification_object->msg_name);
        delete [] d->notification_object->msg_iov;
        delete d->notification_object;
    }
    delete d_ptr;
}

void systemdNotify::notify(const QByteArray &data)
{
    Q_D(systemdNotify);
    Q_ASSERT(d->notification_fd);

    struct msghdr *msghdr = d->notification_object;
    struct iovec *iovec = msghdr->msg_iov;

    iovec[0].iov_base = const_cast<char *>("STATUS=");
    iovec[0].iov_len = 7;

    iovec[1].iov_base = const_cast<char *>(data.constData());
    iovec[1].iov_len = data.size();

    iovec[2].iov_base = const_cast<char *>("\n");
    iovec[2].iov_len = 1;

    msghdr->msg_iovlen = 3;

    if (sendmsg(d->notification_fd, msghdr, 0) < 0) {
        qCWarning(WSGI_SYSTEMD, "sendmsg()");
    }
}

void systemdNotify::ready()
{
    Q_D(systemdNotify);
    Q_ASSERT(d->notification_fd);

    struct msghdr *msghdr = d->notification_object;
    struct iovec *iovec = msghdr->msg_iov;

    iovec[0].iov_base = const_cast<char *>("STATUS=cutelyst-wsgi is ready\nREADY=1\n");
    iovec[0].iov_len = 38;

    msghdr->msg_iovlen = 1;

    if (sendmsg(d->notification_fd, msghdr, 0) < 0) {
        qCWarning(WSGI_SYSTEMD, "sendmsg()");
    }
}

void systemdNotify::install_systemd_notifier(WSGI *wsgi)
{
    if (!qEnvironmentVariableIsSet("NOTIFY_SOCKET")) {
        return;
    }

    const QByteArray notifySocket = qgetenv("NOTIFY_SOCKET");
    qInfo(WSGI_SYSTEMD) << "systemd notify detected" << notifySocket;
    auto notify = new systemdNotify(notifySocket.constData(), wsgi);
    connect(wsgi, &WSGI::ready, notify, &systemdNotify::ready);
}

int fd_cloexec(int fd, bool cloexec) {
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
    int n = listenFDS.toInt(&ok);
    if (!ok) {
        return 0;
    }

    Q_ASSERT(SD_LISTEN_FDS_START < INT_MAX);
    if (n <= 0 || n > INT_MAX - SD_LISTEN_FDS_START) {
        return  -EINVAL;
    }

    qCInfo(WSGI_SYSTEMD, "systemd socket activation detected");

    int r = 0;
    for (int fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; fd ++) {
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
