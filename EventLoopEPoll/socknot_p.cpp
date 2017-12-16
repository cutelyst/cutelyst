#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QPointer>
#include <QtCore/QSocketNotifier>
#include <sys/epoll.h>
#include <errno.h>
#include "eventdispatcher_epoll_p.h"

void EventDispatcherEPollPrivate::registerSocketNotifier(QSocketNotifier* notifier)
{
    Q_ASSERT(notifier != 0);
    Q_ASSUME(notifier != 0);

    int events = 0;
    QSocketNotifier** n = 0;
    int fd = static_cast<int>(notifier->socket());

    epoll_event e;
//    e.data.fd = fd;

    HandleData* data;
    HandleHash::Iterator it = m_handles.find(fd);

    if (it == m_handles.end()) {
        data        = new HandleData;
        data->fd = fd;
        e.data.ptr = data;
        data->type  = htSocketNotifier;
        data->sni.r = 0;
        data->sni.w = 0;
        data->sni.x = 0;

        switch (notifier->type()) {
        case QSocketNotifier::Read:      events = EPOLLIN;  n = &data->sni.r; break;
        case QSocketNotifier::Write:     events = EPOLLOUT; n = &data->sni.w; break;
        case QSocketNotifier::Exception: events = EPOLLPRI; n = &data->sni.x; break;
        default:
            Q_UNREACHABLE();
        }

        data->sni.events = events;
        e.events         = events;
        *n               = notifier;

        int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &e);
        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
            delete data;
            return;
        }

        m_handles.insert(fd, data);
    }
    else {
        data = it.value();
        e.data.ptr = data;

        Q_ASSERT(data->type == htSocketNotifier);
        if (data->type == htSocketNotifier) {
            switch (notifier->type()) {
            case QSocketNotifier::Read:      events = EPOLLIN;  n = &data->sni.r; break;
            case QSocketNotifier::Write:     events = EPOLLOUT; n = &data->sni.w; break;
            case QSocketNotifier::Exception: events = EPOLLPRI; n = &data->sni.x; break;
            default:
                Q_UNREACHABLE();
            }

            Q_ASSERT(n != 0);
            if (Q_UNLIKELY(*n != 0)) {
                qWarning("%s: cannot add two socket notifiers of the same type for the same descriptor", Q_FUNC_INFO);
                return;
            }

            Q_ASSERT((data->sni.events & events) == 0);

            data->sni.events |= events;
            e.events          = data->sni.events;
            *n                = notifier;

            int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
            if (Q_UNLIKELY(res != 0)) {
                qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
                return;
            }
        }
        else {
            Q_UNREACHABLE();
        }
    }

    Q_ASSERT(!m_notifiers.contains(notifier));
    m_notifiers.insert(notifier, data);
}

void EventDispatcherEPollPrivate::unregisterSocketNotifier(QSocketNotifier* notifier)
{
    Q_ASSERT(notifier != 0);
    Q_ASSUME(notifier != 0);

    auto it = m_notifiers.find(notifier);
    if (Q_LIKELY(it != m_notifiers.end())) {
        HandleData* info = it.value();
        int fd           = static_cast<int>(notifier->socket());

        m_notifiers.erase(it); // Hash is not rehashed

        auto hi = m_handles.find(fd);
        Q_ASSERT(hi != m_handles.end());

        struct epoll_event e;
//        e.data.fd = fd;
        e.data.ptr = info;

        if (info->sni.r == notifier) {
            info->sni.events &= ~EPOLLIN;
            info->sni.r       = 0;
        }
        else if (info->sni.w == notifier) {
            info->sni.events &= ~EPOLLOUT;
            info->sni.w       = 0;
        }
        else if (info->sni.x == notifier) {
            info->sni.events &= ~EPOLLPRI;
            info->sni.x       = 0;
        }
        else {
            qFatal("%s: internal error: cannot find socket notifier", Q_FUNC_INFO);
        }

        e.events = info->sni.events;

        int res;

        if (info->sni.r || info->sni.w || info->sni.x) {
            res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
        }
        else {
            res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &e);
            if (Q_UNLIKELY(res != 0 && EBADF == errno)) {
                res = 0;
            }

            m_handles.erase(hi);
            delete info;
        }

        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
        }
    }
}

void EventDispatcherEPollPrivate::socket_notifier_callback(const SocketNotifierInfo& n, int events)
{
    QEvent e(QEvent::SockAct);

    QPointer<QSocketNotifier> r(n.r);
    QPointer<QSocketNotifier> w(n.w);
    QPointer<QSocketNotifier> x(n.x);

    if (r && (events & EPOLLIN)) {
        QCoreApplication::sendEvent(r, &e);
    }

    if (w && (events & EPOLLOUT)) {
        QCoreApplication::sendEvent(w, &e);
    }

    if (x && (events & EPOLLPRI)) {
        QCoreApplication::sendEvent(x, &e);
    }
}

bool EventDispatcherEPollPrivate::disableSocketNotifiers(bool disable)
{
    epoll_event e;

    auto it = m_notifiers.constBegin();
    while (it != m_notifiers.constEnd()) {
        QSocketNotifier* notifier = it.key();
        HandleData* info          = it.value();
        int fd                    = static_cast<int>(notifier->socket());

        e.events  = disable ? 0 : info->sni.events;
//        e.data.fd = fd;
        e.data.ptr = info;
        int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
        }

        ++it;
    }

    return true;
}
