/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EVENTDISPATCHER_EPOLL_H
#define EVENTDISPATCHER_EPOLL_H

#include <QtCore/QAbstractEventDispatcher>

class EventDispatcherEPollPrivate;

#if defined(cutelyst_qt_eventloop_epoll_EXPORTS)
#    define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_EXPORT
#else
#    define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_IMPORT
#endif

class CUTELYST_EVENTLOOP_EPOLL_EXPORT EventDispatcherEPoll final : public QAbstractEventDispatcher
{
    Q_OBJECT
public:
    explicit EventDispatcherEPoll(QObject *parent = nullptr);
    virtual ~EventDispatcherEPoll() override;

    void reinstall();

    bool processEvents(QEventLoop::ProcessEventsFlags flags) override;

    void registerSocketNotifier(QSocketNotifier *notifier) override;
    void unregisterSocketNotifier(QSocketNotifier *notifier) override;

    bool unregisterTimer(int timerId) override;
    bool unregisterTimers(QObject *object) override;
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const override;
    int remainingTime(int timerId) override;

    void wakeUp() override;
    void interrupt() override;

    void registerTimer(int timerId,
                       qint64 interval,
                       Qt::TimerType timerType,
                       QObject *object) override;

private:
    Q_DISABLE_COPY(EventDispatcherEPoll)
    Q_DECLARE_PRIVATE(EventDispatcherEPoll)

    EventDispatcherEPollPrivate *d_ptr;
};

#endif // EVENTDISPATCHER_EPOLL_H
