#include <QtCore/QCoreApplication>
#include <QPointer>
#include <QSocketNotifier>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <stdlib.h>
#include <errno.h>
#include "eventdispatcher_epoll.h"
#include "eventdispatcher_epoll_p.h"

EventDispatcherEPollPrivate::EventDispatcherEPollPrivate(EventDispatcherEPoll* const q)
    : q_ptr(q)
{
    createEpoll();
}

EventDispatcherEPollPrivate::~EventDispatcherEPollPrivate(void)
{
    close(m_event_fd);
    close(m_epoll_fd);

    auto it = m_handles.constBegin();
    while (it != m_handles.constEnd()) {
        delete it.value();
        ++it;
    }
}

void EventDispatcherEPollPrivate::createEpoll()
{
    m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (Q_UNLIKELY(-1 == m_epoll_fd)) {
        qErrnoWarning("epoll_create1() failed");
        abort();
    }

    m_event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (Q_UNLIKELY(-1 == m_event_fd)) {
        qErrnoWarning("eventfd() failed");
        abort();
    }

    struct epoll_event e;
    e.events  = EPOLLIN;
    auto data = new EventFdInfo;
    data->epPriv = this;
    data->fd = m_event_fd;
    e.data.ptr = data;
    if (Q_UNLIKELY(-1 == epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_event_fd, &e))) {
        qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
    }
}

bool EventDispatcherEPollPrivate::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_Q(EventDispatcherEPoll);

    const bool exclude_notifiers = (flags & QEventLoop::ExcludeSocketNotifiers);
    const bool exclude_timers    = (flags & QEventLoop::X11ExcludeTimers);

    exclude_notifiers && disableSocketNotifiers(true);
    exclude_timers    && disableTimers(true);

    m_interrupt = false;
    Q_EMIT q->awake();

    bool result = q->hasPendingEvents();

    QCoreApplication::sendPostedEvents();

    bool can_wait =
            !m_interrupt
            && (flags & QEventLoop::WaitForMoreEvents)
            && !result
            ;

    int n_events = 0;

    if (!m_interrupt) {
        int timeout = 0;

        if (!exclude_timers && !m_zero_timers.isEmpty()) {
            auto it = m_zero_timers.constBegin();
            while (it != m_zero_timers.constEnd()) {
                ZeroTimer *data = it.value();
                ++data->refs;
                ++it;
            }

            it = m_zero_timers.constBegin();
            while (it != m_zero_timers.constEnd()) {
                ZeroTimer *data = it.value();
                if (data->refs && data->active) {
                    data->active = false;

                    QTimerEvent event(it.key());
                    QCoreApplication::sendEvent(data->object, &event);

                    result = true;
                    if (!data->active) {
                        data->active = true;
                    }
                }

                if (--data->refs == 0) {
                    delete data;
                }

                ++it;
            }
        }

        if (can_wait && !result) {
            Q_EMIT q->aboutToBlock();
            timeout = -1;
        }

        struct epoll_event events[1024];
        do {
            n_events = epoll_wait(m_epoll_fd, events, 1024, timeout);
        } while (Q_UNLIKELY(-1 == n_events && errno == EINTR));

        for (int i = 0; i < n_events; ++i) {
            struct epoll_event &e = events[i];
            auto data = static_cast<EpollAbastractEvent*>(e.data.ptr);
            ++data->refs;
        }

        for (int i = 0; i < n_events; ++i) {
            struct epoll_event &e = events[i];
            auto data = static_cast<EpollAbastractEvent*>(e.data.ptr);
            if (data->refs) {
                data->process(e);
            }

            if (--data->refs == 0) {
                delete data;
            }
        }
    }

    exclude_notifiers && disableSocketNotifiers(false);
    exclude_timers    && disableTimers(false);

    return result || n_events > 0;
}

void EventDispatcherEPollPrivate::wake_up_handler()
{
    eventfd_t value;
    int res;
    do {
        res = eventfd_read(m_event_fd, &value);
    } while (Q_UNLIKELY(-1 == res && EINTR == errno));

    if (Q_UNLIKELY(-1 == res)) {
        qErrnoWarning("%s: eventfd_read() failed", Q_FUNC_INFO);
    }

    if (Q_UNLIKELY(!m_wakeups.testAndSetRelease(1, 0))) {
        qCritical("%s: internal error, testAndSetRelease(1, 0) failed!", Q_FUNC_INFO);
    }
}

void SocketNotifierInfo::process(epoll_event &ee)
{
    QEvent e(QEvent::SockAct);

    if (r && (ee.events & EPOLLIN)) {
        QCoreApplication::sendEvent(r, &e);
    }

    if (w && (ee.events & EPOLLOUT)) {
        QCoreApplication::sendEvent(w, &e);
    }

    if (x && (ee.events & EPOLLPRI)) {
        QCoreApplication::sendEvent(x, &e);
    }
}

void EventFdInfo::process(epoll_event &e)
{
    if (Q_LIKELY(e.events & EPOLLIN)) {
        epPriv->wake_up_handler();
    }
}

void TimerInfo::process(epoll_event &e)
{
    Q_UNUSED(e)

    uint64_t value;
    int res;
    do {
        res = read(fd, &value, sizeof(value));
    } while (-1 == res && EINTR == errno);

    if (Q_UNLIKELY(-1 == res)) {
        qErrnoWarning("%s: read() failed", Q_FUNC_INFO);
    }

    QTimerEvent event(timerId);
    QCoreApplication::sendEvent(object, &event);

    // Check if we are NOT going to be deleted
    if (refs > 1) {
        struct timeval now;
        struct timeval delta;
        struct itimerspec spec;

        spec.it_interval.tv_sec  = 0;
        spec.it_interval.tv_nsec = 0;

        gettimeofday(&now, 0);
        EventDispatcherEPollPrivate::calculateNextTimeout(this, now, delta);
        TIMEVAL_TO_TIMESPEC(&delta, &spec.it_value);
        if (0 == spec.it_value.tv_sec && 0 == spec.it_value.tv_nsec) {
            spec.it_value.tv_nsec = 500;
        }

        if (-1 == timerfd_settime(fd, 0, &spec, 0)) {
            qErrnoWarning("%s: timerfd_settime() failed", Q_FUNC_INFO);
        }
    }
}

void ZeroTimer::process(epoll_event &e)
{
    Q_UNUSED(e)
}
