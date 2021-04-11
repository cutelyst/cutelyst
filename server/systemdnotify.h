/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SYSTEMDNOTIFY_H
#define SYSTEMDNOTIFY_H

#include <QByteArray>
#include <QObject>

namespace Cutelyst {

class Server;
class systemdNotifyPrivate;
class systemdNotify : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(systemdNotify)
public:
    explicit systemdNotify(const char *systemd_socket, QObject *parent = nullptr);
    ~systemdNotify();

    void notify(const QByteArray &data);
    void ready();

    static void install_systemd_notifier(Server *wsgi);

    static std::vector<int> listenFds(bool unsetEnvironment = true);

private:
    systemdNotifyPrivate *d_ptr;
};

}

#endif // SYSTEMDNOTIFY_H
