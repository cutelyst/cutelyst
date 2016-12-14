#include <QtCore/QPair>
#include <QtCore/QSocketNotifier>
#include <QtCore/QThread>
#include "eventdispatcher_epoll.h"
#include "eventdispatcher_epoll_p.h"

EventDispatcherEPoll::EventDispatcherEPoll(QObject* parent)
    : QAbstractEventDispatcher(parent), d_ptr(new EventDispatcherEPollPrivate(this))
{
}

EventDispatcherEPoll::~EventDispatcherEPoll(void)
{
    delete d_ptr;
}

bool EventDispatcherEPoll::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(EventDispatcherEPoll);
    return d->processEvents(flags);
}

bool EventDispatcherEPoll::hasPendingEvents(void)
{
    extern uint qGlobalPostedEventsCount();
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
        QObject* object
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
    }
    else {
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

bool EventDispatcherEPoll::unregisterTimers(QObject* object)
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

QList<QAbstractEventDispatcher::TimerInfo> EventDispatcherEPoll::registeredTimers(QObject* object) const
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

void EventDispatcherEPoll::wakeUp(void)
{
    Q_D(EventDispatcherEPoll);
    d->wakeup();
}

void EventDispatcherEPoll::interrupt(void)
{
    Q_D(EventDispatcherEPoll);
    d->m_interrupt = true;
    wakeUp();
}

void EventDispatcherEPoll::flush(void)
{
}

#include "moc_eventdispatcher_epoll.cpp"
