/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "systemdnotify.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <QDebug>

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
        qWarning("socket()");
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
        qWarning("sendmsg()");
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
        qWarning("sendmsg()");
    }
}
