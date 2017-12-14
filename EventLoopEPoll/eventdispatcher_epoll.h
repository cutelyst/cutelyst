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
    explicit EventDispatcherEPoll(QObject* parent = nullptr);
    virtual ~EventDispatcherEPoll() override;

    virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
    virtual bool hasPendingEvents() override;

    virtual void registerSocketNotifier(QSocketNotifier* notifier) override;
    virtual void unregisterSocketNotifier(QSocketNotifier* notifier) override;

    virtual void registerTimer(
            int timerId,
            int interval,
            Qt::TimerType timerType,
            QObject* object
            ) override;

    virtual bool unregisterTimer(int timerId) override;
    virtual bool unregisterTimers(QObject* object) override;
    virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const override;
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
