/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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

    void sendStatus(const QByteArray &data);
    void sendWatchdog(const QByteArray &data);
    void sendReady(const QByteArray &data);

    // Returns the usec if > 0 else it is disabled
    // Set unset to true before forking
    static int sd_watchdog_enabled(bool unset);

    static bool is_systemd_notify_available();

    static std::vector<int> listenFds(bool unsetEnvironment = true);

private:
    systemdNotifyPrivate *d_ptr;
};

} // namespace Cutelyst

#endif // SYSTEMDNOTIFY_H
