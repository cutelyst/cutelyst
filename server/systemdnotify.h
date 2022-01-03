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

class systemdNotifyPrivate;
class systemdNotify : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(systemdNotify)
public:
    explicit systemdNotify(QObject *parent = nullptr);
    ~systemdNotify();

    int watchdogUSec() const;

    bool setWatchdog(bool enable, int usec = 0);

    void notify(const QByteArray &data);
    void ready();

    // Returns the usec if > 0 else it is disabled
    // Set unset to true before forking
    static int sd_watchdog_enabled(bool unset);

    static bool is_systemd_notify_available();

    static std::vector<int> listenFds(bool unsetEnvironment = true);

private:
    systemdNotifyPrivate *d_ptr;
};

}

#endif // SYSTEMDNOTIFY_H
