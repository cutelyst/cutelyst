#include <QtCore/QCoreApplication>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
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
    e.data.fd = m_event_fd;
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

        if (!exclude_timers && m_zero_timers.size() > 0) {
            auto it = m_zero_timers.begin();
            auto end = m_zero_timers.end();
            while (it != end) {
                ZeroTimer &data = it.value();
                if (data.active) {
                    data.active = false;

                    QTimerEvent event(it.key());
                    // Single shot timers were crashing here, with suposedly invalid pointers
                    // thus a regular timer is being registered
                    QCoreApplication::sendEvent(data.object, &event);
                    result = true;

                    // I believe the send event might change the m_zero_timers
                    // hash it's the only explanation to this:
                    auto i = m_zero_timers.find(it.key());
                    if (i != m_zero_timers.end()) {
                        ZeroTimer& data = it.value();
                        if (!data.active) {
                            data.active = true;
                        }
                    }
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

        for (int i=0; i<n_events; ++i) {
            struct epoll_event& e = events[i];
            int fd                = e.data.fd;
            if (fd == m_event_fd) {
                if (Q_LIKELY(e.events & EPOLLIN)) {
                    wake_up_handler();
                }
            }
            else {
                auto it = m_handles.constFind(fd);
                if (Q_LIKELY(it != m_handles.constEnd())) {
                    HandleData* data = it.value();
                    switch (data->type) {
                    case htSocketNotifier:
                        EventDispatcherEPollPrivate::socket_notifier_callback(data->sni, e.events);
                        break;

                    case htTimer:
                        timer_callback(data->ti);
                        break;

                    default:
                        Q_UNREACHABLE();
                    }
                }
            }
        }
    }

    exclude_notifiers && disableSocketNotifiers(false);
    exclude_timers    && disableTimers(false);

    return result || n_events > 0;
}

void EventDispatcherEPollPrivate::wake_up_handler(void)
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

void EventDispatcherEPollPrivate::wakeup(void)
{
    if (m_wakeups.testAndSetAcquire(0, 1))
    {
        const eventfd_t value = 1;
        int res;

        do {
            res = eventfd_write(m_event_fd, value);
        } while (Q_UNLIKELY(-1 == res && EINTR == errno));

        if (Q_UNLIKELY(-1 == res)) {
            qErrnoWarning("%s: eventfd_write() failed", Q_FUNC_INFO);
        }
    }
}
