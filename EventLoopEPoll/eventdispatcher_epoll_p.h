#ifndef EVENTDISPATCHER_EPOLL_P_H
#define EVENTDISPATCHER_EPOLL_P_H

#include <qplatformdefs.h>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QHash>

#include <QtCore/QAtomicInt>

class EpollAbastractEvent
{
public:
    explicit EpollAbastractEvent(int _fd = 0) : fd(_fd) {}
    virtual ~EpollAbastractEvent() {}

    int fd;
    int refs = 1;

    virtual void process(struct epoll_event &e) = 0;
};

class EventDispatcherEPollPrivate;
class EventFdInfo : public EpollAbastractEvent
{
public:
    EventFdInfo(int _fd, EventDispatcherEPollPrivate *prv) : EpollAbastractEvent(_fd), epPriv(prv) {}

    virtual void process(struct epoll_event &e);

    EventDispatcherEPollPrivate *epPriv;
};

class SocketNotifierInfo : public EpollAbastractEvent
{
public:
    SocketNotifierInfo(int _fd) : EpollAbastractEvent(_fd) { }

    virtual void process(struct epoll_event &ee);

    QSocketNotifier *r = nullptr;
    QSocketNotifier *w = nullptr;
    QSocketNotifier *x = nullptr;
    int events;
};

class ZeroTimer : public EpollAbastractEvent
{
public:
    ZeroTimer(QObject *obj) : object(obj) {}

    virtual void process(struct epoll_event &e);

    QObject *object;
    bool active = true;
};

class TimerInfo : public EpollAbastractEvent
{
public:
    TimerInfo(int fd, int _timerId, int _interval, QObject *obj)
        : EpollAbastractEvent(fd), object(obj), timerId(_timerId), interval(_interval) {}

    virtual void process(struct epoll_event &e);

    QObject *object;
    struct timeval when;
    int timerId;
    int interval;
    Qt::TimerType type;
};

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
    typedef QHash<int, ZeroTimer*> ZeroTimerHash;

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
