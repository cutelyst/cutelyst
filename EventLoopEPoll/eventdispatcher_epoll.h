#ifndef EVENTDISPATCHER_EPOLL_H
#define EVENTDISPATCHER_EPOLL_H

#include <QtCore/QAbstractEventDispatcher>

class EventDispatcherEPollPrivate;

#if defined(cutelyst_qt_eventloop_epoll_EXPORTS)
#  define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_EVENTLOOP_EPOLL_EXPORT Q_DECL_IMPORT
#endif

class CUTELYST_EVENTLOOP_EPOLL_EXPORT EventDispatcherEPoll : public QAbstractEventDispatcher {
    Q_OBJECT
public:
    explicit EventDispatcherEPoll(QObject* parent = 0);
    virtual ~EventDispatcherEPoll(void);

    virtual bool processEvents(QEventLoop::ProcessEventsFlags flags);
    virtual bool hasPendingEvents(void);

    virtual void registerSocketNotifier(QSocketNotifier* notifier);
    virtual void unregisterSocketNotifier(QSocketNotifier* notifier);

    virtual void registerTimer(
            int timerId,
            int interval,
            Qt::TimerType timerType,
            QObject* object
            );

    virtual bool unregisterTimer(int timerId);
    virtual bool unregisterTimers(QObject* object);
    virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const;
    virtual int remainingTime(int timerId);

    virtual void wakeUp(void);
    virtual void interrupt(void);
    virtual void flush(void);

    void postFork();

private:
    Q_DISABLE_COPY(EventDispatcherEPoll)
    Q_DECLARE_PRIVATE(EventDispatcherEPoll)

    EventDispatcherEPollPrivate *d_ptr;
};

#endif // EVENTDISPATCHER_EPOLL_H
