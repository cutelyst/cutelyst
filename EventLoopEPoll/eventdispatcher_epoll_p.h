#ifndef EVENTDISPATCHER_EPOLL_P_H
#define EVENTDISPATCHER_EPOLL_P_H

#include <qplatformdefs.h>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QHash>

#include <QtCore/QAtomicInt>

struct ZeroTimer {
    QObject* object;
    bool active;
};

class EpollAbastractEvent
{
public:
    virtual ~EpollAbastractEvent() {}

    int fd;

    virtual void process(struct epoll_event &e) = 0;
};

class EventDispatcherEPollPrivate;
class EventFdInfo : public EpollAbastractEvent
{
public:
    EventDispatcherEPollPrivate *epPriv;
    virtual void process(struct epoll_event &e);
};

class SocketNotifierInfo : public EpollAbastractEvent
{
public:
    QSocketNotifier *r = nullptr;
    QSocketNotifier *w = nullptr;
    QSocketNotifier *x = nullptr;
    int events;
    virtual void process(struct epoll_event &ee);
};

class TimerInfo : public EpollAbastractEvent
{
public:
    QObject* object;
    struct timeval when;
    int timerId;
    int interval;
    Qt::TimerType type;
    EventDispatcherEPollPrivate *epPriv;
    virtual void process(struct epoll_event &e);
};

//Q_DECLARE_TYPEINFO(SocketNotifierInfo, Q_PRIMITIVE_TYPE);
//Q_DECLARE_TYPEINFO(TimerInfo, Q_PRIMITIVE_TYPE);
//Q_DECLARE_TYPEINFO(HandleData, Q_PRIMITIVE_TYPE);

class EventDispatcherEPoll;

class Q_DECL_HIDDEN EventDispatcherEPollPrivate {
public:
    EventDispatcherEPollPrivate(EventDispatcherEPoll* const q);
    ~EventDispatcherEPollPrivate(void);
    void createEpoll();
    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    void registerSocketNotifier(QSocketNotifier* notifier);
    void unregisterSocketNotifier(QSocketNotifier* notifier);
    void registerTimer(int timerId, int interval, Qt::TimerType type, QObject* object);
    void registerZeroTimer(int timerId, QObject* object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject* object);
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const;
    int remainingTime(int timerId) const;
    void wake_up_handler();

    static void calculateNextTimeout(TimerInfo* info, const struct timeval& now, struct timeval& delta);

    typedef QHash<int, EpollAbastractEvent*> HandleHash;
    typedef QHash<int, TimerInfo*> TimerHash;
    typedef QHash<QSocketNotifier*, SocketNotifierInfo*> SocketNotifierHash;
    typedef QHash<int, ZeroTimer> ZeroTimerHash;

private:
    Q_DISABLE_COPY(EventDispatcherEPollPrivate)
    Q_DECLARE_PUBLIC(EventDispatcherEPoll)
    EventDispatcherEPoll* const q_ptr;

    int m_epoll_fd = -1;
    int m_event_fd = -1;
    bool m_interrupt = false;
    QAtomicInt m_wakeups;
    HandleHash m_handles;
    SocketNotifierHash m_notifiers;
    TimerHash m_timers;
    ZeroTimerHash m_zero_timers;

    bool disableSocketNotifiers(bool disable);
    bool disableTimers(bool disable);
};

#endif // EVENTDISPATCHER_EPOLL_P_H
