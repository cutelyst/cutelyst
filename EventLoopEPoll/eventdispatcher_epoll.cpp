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
#include <QtCore/QSocketNotifier>
#include <QtCore/QThread>

#include <sys/eventfd.h>

#include "eventdispatcher_epoll.h"
#include "eventdispatcher_epoll_p.h"

EventDispatcherEPoll::EventDispatcherEPoll(QObject* parent)
    : QAbstractEventDispatcher(parent), d_ptr(new EventDispatcherEPollPrivate(this))
{
}

EventDispatcherEPoll::~EventDispatcherEPoll()
{
    delete d_ptr;
}

bool EventDispatcherEPoll::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(EventDispatcherEPoll);
    return d->processEvents(flags);
}

extern uint qGlobalPostedEventsCount();

bool EventDispatcherEPoll::hasPendingEvents()
{
    return qGlobalPostedEventsCount() > 0;
}

void EventDispatcherEPoll::registerSocketNotifier(QSocketNotifier* notifier)
{
#ifndef QT_NO_DEBUG
    if (notifier->socket() < 0) {
        qWarning("QSocketNotifier: Internal error: sockfd < 0");
        return;
    }

    if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }
#endif

    Q_D(EventDispatcherEPoll);
    d->registerSocketNotifier(notifier);
}

void EventDispatcherEPoll::unregisterSocketNotifier(QSocketNotifier* notifier)
{
#ifndef QT_NO_DEBUG
    if (notifier->socket() < 0) {
        qWarning("QSocketNotifier: Internal error: sockfd < 0");
        return;
    }

    if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
        return;
    }
#endif

    Q_D(EventDispatcherEPoll);
    d->unregisterSocketNotifier(notifier);
}

void EventDispatcherEPoll::registerTimer(
        int timerId,
        int interval,
        Qt::TimerType timerType,
        QObject *object
        )
{
#ifndef QT_NO_DEBUG
    if (timerId < 1 || interval < 0 || !object) {
        qWarning("%s: invalid arguments", Q_FUNC_INFO);
        return;
    }

    if (object->thread() != thread() && thread() != QThread::currentThread()) {
        qWarning("%s: timers cannot be started from another thread", Q_FUNC_INFO);
        return;
    }
#endif

    Q_D(EventDispatcherEPoll);
    if (interval) {
        d->registerTimer(timerId, interval, timerType, object);
    } else {
        d->registerZeroTimer(timerId, object);
    }
}

bool EventDispatcherEPoll::unregisterTimer(int timerId)
{
#ifndef QT_NO_DEBUG
    if (timerId < 1) {
        qWarning("%s: invalid arguments", Q_FUNC_INFO);
        return false;
    }

    if (thread() != QThread::currentThread()) {
        qWarning("%s: timers cannot be stopped from another thread", Q_FUNC_INFO);
        return false;
    }
#endif

    Q_D(EventDispatcherEPoll);
    return d->unregisterTimer(timerId);
}

bool EventDispatcherEPoll::unregisterTimers(QObject *object)
{
#ifndef QT_NO_DEBUG
    if (!object) {
        qWarning("%s: invalid arguments", Q_FUNC_INFO);
        return false;
    }

    if (object->thread() != thread() && thread() != QThread::currentThread()) {
        qWarning("%s: timers cannot be stopped from another thread", Q_FUNC_INFO);
        return false;
    }
#endif

    Q_D(EventDispatcherEPoll);
    return d->unregisterTimers(object);
}

QList<QAbstractEventDispatcher::TimerInfo> EventDispatcherEPoll::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("%s: invalid argument", Q_FUNC_INFO);
        return QList<QAbstractEventDispatcher::TimerInfo>();
    }

    Q_D(const EventDispatcherEPoll);
    return d->registeredTimers(object);
}

int EventDispatcherEPoll::remainingTime(int timerId)
{
    Q_D(const EventDispatcherEPoll);
    return d->remainingTime(timerId);
}

void EventDispatcherEPoll::wakeUp()
{
    Q_D(EventDispatcherEPoll);

    if (d->m_wakeups.testAndSetAcquire(0, 1)) {
        const eventfd_t value = 1;
        int res;

        do {
            res = eventfd_write(d->m_event_fd, value);
        } while (Q_UNLIKELY(-1 == res && EINTR == errno));

        if (Q_UNLIKELY(-1 == res)) {
            qErrnoWarning("%s: eventfd_write() failed", Q_FUNC_INFO);
        }
    }
}

void EventDispatcherEPoll::interrupt()
{
    Q_D(EventDispatcherEPoll);
    d->m_interrupt = true;
    wakeUp();
}

void EventDispatcherEPoll::flush()
{
    Q_D(EventDispatcherEPoll);
    d->createEpoll();
}

#include "moc_eventdispatcher_epoll.cpp"
