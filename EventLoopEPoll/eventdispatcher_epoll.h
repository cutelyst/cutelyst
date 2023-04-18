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

    virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) override;

    virtual void registerSocketNotifier(QSocketNotifier *notifier) override;
    virtual void unregisterSocketNotifier(QSocketNotifier *notifier) override;

    virtual bool unregisterTimer(int timerId) override;
    virtual bool unregisterTimers(QObject *object) override;
    virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const override;
    virtual int remainingTime(int timerId) override;

    virtual void wakeUp() override;
    virtual void interrupt() override;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    virtual bool hasPendingEvents() override;
    virtual void registerTimer(
        int timerId,
        int interval,
        Qt::TimerType timerType,
        QObject *object) override;
    virtual void flush() override;
#else
    virtual void registerTimer(int timerId, qint64 interval, Qt::TimerType timerType, QObject *object) override;
#endif

private:
    Q_DISABLE_COPY(EventDispatcherEPoll)
    Q_DECLARE_PRIVATE(EventDispatcherEPoll)

    EventDispatcherEPollPrivate *d_ptr;
};

#endif // EVENTDISPATCHER_EPOLL_H
