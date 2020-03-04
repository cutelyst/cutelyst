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
#ifndef EVENTDISPATCHER_EPOLL_H
#define EVENTDISPATCHER_EPOLL_H

#include <QtCore/QAbstractEventDispatcher>

class EventDispatcherEPollPrivate;

#if defined(cutelyst_qt_eventloop_epoll_EXPORTS)
#  define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_IMPORT
#endif

class CUTELYST_EVENTLOOP_EPOLL_EXPORT EventDispatcherEPoll final : public QAbstractEventDispatcher {
    Q_OBJECT
public:
    explicit EventDispatcherEPoll(QObject *parent = nullptr);
    virtual ~EventDispatcherEPoll() override;

    virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
    virtual bool hasPendingEvents() override;

    virtual void registerSocketNotifier(QSocketNotifier *notifier) override;
    virtual void unregisterSocketNotifier(QSocketNotifier *notifier) override;

    virtual void registerTimer(
            int timerId,
            int interval,
            Qt::TimerType timerType,
            QObject *object
            ) override;

    virtual bool unregisterTimer(int timerId) override;
    virtual bool unregisterTimers(QObject *object) override;
    virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const override;
    virtual int remainingTime(int timerId) override;

    virtual void wakeUp() override;
    virtual void interrupt() override;
    virtual void flush() override;

private:
    Q_DISABLE_COPY(EventDispatcherEPoll)
    Q_DECLARE_PRIVATE(EventDispatcherEPoll)

    EventDispatcherEPollPrivate *d_ptr;
};

#endif // EVENTDISPATCHER_EPOLL_H
