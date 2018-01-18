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
#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "eventdispatcher_epoll_p.h"

namespace {

static void calculateCoarseTimerTimeout(TimerInfo *info, const struct timeval &now, struct timeval &when)
{
    Q_ASSERT(info->interval > 20);
    // The coarse timer works like this:
    //  - interval under 40 ms: round to even
    //  - between 40 and 99 ms: round to multiple of 4
    //  - otherwise: try to wake up at a multiple of 25 ms, with a maximum error of 5%
    //
    // We try to wake up at the following second-fraction, in order of preference:
    //    0 ms
    //  500 ms
    //  250 ms or 750 ms
    //  200, 400, 600, 800 ms
    //  other multiples of 100
    //  other multiples of 50
    //  other multiples of 25
    //
    // The objective is to make most timers wake up at the same time, thereby reducing CPU wakeups.

    int interval     = info->interval;
    int msec         = static_cast<int>(info->when.tv_usec / 1000);
    int max_rounding = interval / 20; // 5%
    when             = info->when;

    if (interval < 100 && (interval % 25) != 0) {
        if (interval < 50) {
            uint round_up = ((msec % 50) >= 25) ? 1 : 0;
            msec = ((msec >> 1) | round_up) << 1;
        }
        else {
            uint round_up = ((msec % 100) >= 50) ? 1 : 0;
            msec = ((msec >> 2) | round_up) << 2;
        }
    }
    else {
        int min = qMax(0, msec - max_rounding);
        int max = qMin(1000, msec + max_rounding);

        bool done = false;

        // take any round-to-the-second timeout
        if (min == 0) {
            msec = 0;
            done = true;
        }
        else if (max == 1000) {
            msec = 1000;
            done = true;
        }

        if (!done) {
            int boundary;

            // if the interval is a multiple of 500 ms and > 5000 ms, always round towards a round-to-the-second
            // if the interval is a multiple of 500 ms, round towards the nearest multiple of 500 ms
            if ((interval % 500) == 0) {
                if (interval >= 5000) {
                    msec = msec >= 500 ? max : min;
                    done = true;
                }
                else {
                    boundary = 500;
                }
            }
            else if ((interval % 50) == 0) {
                // same for multiples of 250, 200, 100, 50
                uint tmp = interval / 50;
                if ((tmp % 4) == 0) {
                    boundary = 200;
                }
                else if ((tmp % 2) == 0) {
                    boundary = 100;
                }
                else if ((tmp % 5) == 0) {
                    boundary = 250;
                }
                else {
                    boundary = 50;
                }
            }
            else {
                boundary = 25;
            }

            if (!done) {
                int base   = (msec / boundary) * boundary;
                int middle = base + boundary / 2;
                msec       = (msec < middle) ? qMax(base, min) : qMin(base + boundary, max);
            }
        }
    }

    if (msec == 1000) {
        ++when.tv_sec;
        when.tv_usec = 0;
    }
    else {
        when.tv_usec = msec * 1000;
    }

    if (timercmp(&when, &now, <)) {
        when.tv_sec  += interval / 1000;
        when.tv_usec += (interval % 1000) * 1000;
        if (when.tv_usec > 999999) {
            ++when.tv_sec;
            when.tv_usec -= 1000000;
        }
    }

    Q_ASSERT(timercmp(&now, &when, <=));
}

}

void EventDispatcherEPollPrivate::calculateNextTimeout(TimerInfo *info, const struct timeval &now, struct timeval &delta)
{
    struct timeval tv_interval;
    struct timeval when;
    tv_interval.tv_sec  = info->interval / 1000;
    tv_interval.tv_usec = (info->interval % 1000) * 1000;

    info->when = now;
    if (info->interval) {
        qlonglong tnow  = (qlonglong(now.tv_sec)        * 1000) + (now.tv_usec        / 1000);
        qlonglong twhen = (qlonglong(info->when.tv_sec) * 1000) + (info->when.tv_usec / 1000);

        if (Q_UNLIKELY((info->interval < 1000 && twhen - tnow > 1500) || (info->interval >= 1000 && twhen - tnow > 1.2*info->interval))) {
            info->when = now;
        }
    }

    if (Qt::VeryCoarseTimer == info->type) {
        if (info->when.tv_usec >= 500000) {
            ++info->when.tv_sec;
        }

        info->when.tv_usec = 0;
        info->when.tv_sec += info->interval / 1000;
        if (Q_UNLIKELY(info->when.tv_sec <= now.tv_sec)) {
            info->when.tv_sec = now.tv_sec + info->interval / 1000;
        }

        when = info->when;
    }
    else if (Qt::PreciseTimer == info->type) {
        if (info->interval) {
            timeradd(&info->when, &tv_interval, &info->when);
            if (Q_UNLIKELY(timercmp(&info->when, &now, <))) {
                timeradd(&now, &tv_interval, &info->when);
            }

            when = info->when;
        }
        else {
            when = now;
        }
    }
    else {
        timeradd(&info->when, &tv_interval, &info->when);
        if (Q_UNLIKELY(timercmp(&info->when, &now, <))) {
            timeradd(&now, &tv_interval, &info->when);
        }

        calculateCoarseTimerTimeout(info, now, when);
    }

    timersub(&when, &now, &delta);
}

void EventDispatcherEPollPrivate::registerTimer(int timerId, int interval, Qt::TimerType type, QObject *object)
{
    Q_ASSERT(interval > 0);

    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (Q_LIKELY(fd != -1)) {
        struct timeval now;
        gettimeofday(&now, 0);

        auto data = new TimerInfo(fd, timerId, interval, object);
        data->when = now;
        data->type = type;

        if (Qt::CoarseTimer == type) {
            if (interval >= 20000) {
                data->type = Qt::VeryCoarseTimer;
            } else if (interval <= 20) {
                data->type = Qt::PreciseTimer;
            }
        }

        struct timeval delta;
        calculateNextTimeout(data, now, delta);

        struct itimerspec spec;
        spec.it_interval.tv_sec  = 0;
        spec.it_interval.tv_nsec = 0;

        TIMEVAL_TO_TIMESPEC(&delta, &spec.it_value);
        if (0 == spec.it_value.tv_sec && 0 == spec.it_value.tv_nsec) {
            spec.it_value.tv_nsec = 500;
        }

        if (Q_UNLIKELY(-1 == timerfd_settime(fd, 0, &spec, 0))) {
            qErrnoWarning("%s: timerfd_settime() failed", Q_FUNC_INFO);
            delete data;
            close(fd);
            return;
        }

        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = data;

        if (Q_UNLIKELY(-1 == epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event))) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
            delete data;
            close(fd);
            return;
        }

        m_timers.insert(timerId, data);
        m_handles.insert(fd, data);
    } else {
        qErrnoWarning("%s: timerfd_create() failed", Q_FUNC_INFO);
    }
}

void EventDispatcherEPollPrivate::registerZeroTimer(int timerId, QObject *object)
{
    m_zero_timers.insert(timerId, new ZeroTimer(timerId, object));
}

bool EventDispatcherEPollPrivate::unregisterTimer(int timerId)
{
    auto it = m_timers.find(timerId);
    if (it != m_timers.end()) {
        TimerInfo *data = it.value();

        int fd = data->fd;

        if (Q_UNLIKELY(-1 == epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, 0))) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
        }

        close(fd);
        data->deref();

        m_timers.erase(it); // Hash is not rehashed
        m_handles.remove(fd);
        return true;
    } else {
        auto zit = m_zero_timers.find(timerId);
        if (zit != m_zero_timers.end()) {
            ZeroTimer *data = zit.value();
            data->deref();

            m_zero_timers.erase(zit);
            return true;
        }
    }

    return false;
}

bool EventDispatcherEPollPrivate::unregisterTimers(QObject *object)
{
    bool result = false;
    auto it = m_timers.begin();
    while (it != m_timers.end()) {
        TimerInfo *data = it.value();

        if (object == data->object) {
            result = true;
            int fd = data->fd;

            if (Q_UNLIKELY(-1 == epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, 0))) {
                qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
            }

            close(fd);

            data->deref();

            it = m_timers.erase(it); // Hash is not rehashed
            m_handles.remove(fd);
        }
        else {
            ++it;
        }
    }

    auto zit = m_zero_timers.begin();
    while (zit != m_zero_timers.end()) {
        ZeroTimer *data = zit.value();
        if (object == data->object) {
            result = true;
            zit = m_zero_timers.erase(zit);
            data->deref();
        } else {
            ++zit;
        }
    }

    return result;
}

QList<QAbstractEventDispatcher::TimerInfo> EventDispatcherEPollPrivate::registeredTimers(QObject *object) const
{
    QList<QAbstractEventDispatcher::TimerInfo> res;
    res.reserve(m_timers.size() + m_zero_timers.size());

    auto it = m_timers.constBegin();
    while (it != m_timers.constEnd()) {
        TimerInfo *data = it.value();

        if (object == data->object) {
            QAbstractEventDispatcher::TimerInfo ti(it.key(), data->interval, data->type);
            res.append(ti);
        }

        ++it;
    }

    auto zit = m_zero_timers.constBegin();
    while (zit != m_zero_timers.constEnd()) {
        const ZeroTimer *data = zit.value();
        if (object == data->object) {
            QAbstractEventDispatcher::TimerInfo ti(it.key(), 0, Qt::PreciseTimer);
            res.append(ti);
        }

        ++zit;
    }

    return res;
}

int EventDispatcherEPollPrivate::remainingTime(int timerId) const
{
    auto it = m_timers.constFind(timerId);
    if (it != m_timers.constEnd()) {
        TimerInfo *data = it.value();

        struct timeval when;
        struct itimerspec spec;

        if (!data->interval) {
            return -1;
        }

        if (Q_UNLIKELY(-1 == timerfd_gettime(data->fd, &spec))) {
            qErrnoWarning("%s: timerfd_gettime() failed", Q_FUNC_INFO);
            return -1;
        }

        Q_ASSERT(!(spec.it_value.tv_sec == 0 && spec.it_value.tv_nsec == 0));

        TIMESPEC_TO_TIMEVAL(&when, &spec.it_value);
        return static_cast<int>((qulonglong(when.tv_sec) * 1000000 + when.tv_usec) / 1000);
    }

    // For zero timers we return -1 as well

    return -1;
}

bool EventDispatcherEPollPrivate::disableTimers(bool disable)
{
    struct timeval now;
    struct itimerspec spec;

    if (!disable) {
        gettimeofday(&now, 0);
    }
    else {
        spec.it_value.tv_sec  = 0;
        spec.it_value.tv_nsec = 0;
    }

    spec.it_interval.tv_sec  = 0;
    spec.it_interval.tv_nsec = 0;

    auto it = m_timers.constBegin();
    while (it != m_timers.constEnd()) {
        TimerInfo *data = it.value();

        if (!disable) {
            struct timeval delta;
            calculateNextTimeout(data, now, delta);
            TIMEVAL_TO_TIMESPEC(&delta, &spec.it_value);
            if (0 == spec.it_value.tv_sec && 0 == spec.it_value.tv_nsec) {
                spec.it_value.tv_nsec = 500;
            }
        }

        if (Q_UNLIKELY(-1 == timerfd_settime(data->fd, 0, &spec, 0))) {
            qErrnoWarning("%s: timerfd_settime() failed", Q_FUNC_INFO);
        }

        ++it;
    }

    return true;
}
