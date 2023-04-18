/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EVENTDISPATCHER_EPOLL_P_H
#define EVENTDISPATCHER_EPOLL_P_H

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QAtomicInt>
#include <QtCore/QHash>
#include <qplatformdefs.h>

class EpollAbastractEvent
{
public:
    explicit EpollAbastractEvent(int _fd = 0)
        : fd(_fd)
    {
    }
    virtual ~EpollAbastractEvent() = default;

    virtual void process(quint32 events) = 0;

    inline bool canProcess() { return refs > 1; }
    inline void ref() { ++refs; }
    inline void deref()
    {
        if (--refs == 0)
            delete this;
    }

    int fd;
    int refs = 1;
};

class EventDispatcherEPollPrivate;
class EventFdInfo final : public EpollAbastractEvent
{
public:
    EventFdInfo(int _fd, EventDispatcherEPollPrivate *prv)
        : EpollAbastractEvent(_fd)
        , epPriv(prv)
    {
    }

    virtual void process(quint32 events) override;

    EventDispatcherEPollPrivate *epPriv;
};

class SocketNotifierInfo final : public EpollAbastractEvent
{
public:
    SocketNotifierInfo(int _fd)
        : EpollAbastractEvent(_fd)
    {
    }

    virtual void process(quint32 events) override;

    QSocketNotifier *r = nullptr;
    QSocketNotifier *w = nullptr;
    QSocketNotifier *x = nullptr;
    quint32 events     = 0;
};

class ZeroTimer final : public EpollAbastractEvent
{
public:
    ZeroTimer(int _timerId, QObject *obj)
        : object(obj)
        , timerId(_timerId)
    {
    }

    void process(quint32 events) override;

    QObject *object;
    int timerId;
    bool active = true;
};

class TimerInfo final : public EpollAbastractEvent
{
public:
    TimerInfo(int fd, int _timerId, int _interval, QObject *obj)
        : EpollAbastractEvent(fd)
        , object(obj)
        , timerId(_timerId)
        , interval(_interval)
    {
    }

    void process(quint32 events) override;

    QObject *object;
    struct timeval when;
    int timerId;
    int interval;
    Qt::TimerType type;
};

class EventDispatcherEPoll;

class Q_DECL_HIDDEN EventDispatcherEPollPrivate
{
public:
    EventDispatcherEPollPrivate(EventDispatcherEPoll *const q);
    ~EventDispatcherEPollPrivate();
    void createEpoll();
    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);
    void registerTimer(int timerId, int interval, Qt::TimerType type, QObject *object);
    void registerZeroTimer(int timerId, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const;
    int remainingTime(int timerId) const;
    void wake_up_handler();

    static void calculateNextTimeout(TimerInfo *info, const struct timeval &now, struct timeval &delta);

private:
    Q_DISABLE_COPY(EventDispatcherEPollPrivate)
    Q_DECLARE_PUBLIC(EventDispatcherEPoll)
    EventDispatcherEPoll *const q_ptr;

    int m_epoll_fd   = -1;
    int m_event_fd   = -1;
    bool m_interrupt = false;
    EventFdInfo *m_event_fd_info;
    QAtomicInt m_wakeups;
    QHash<int, EpollAbastractEvent *> m_handles;
    QHash<QSocketNotifier *, SocketNotifierInfo *> m_notifiers;
    QHash<int, TimerInfo *> m_timers;
    QHash<int, ZeroTimer *> m_zero_timers;

    bool disableSocketNotifiers(bool disable);
    bool disableTimers(bool disable);
};

#endif // EVENTDISPATCHER_EPOLL_P_H
