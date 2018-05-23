/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QPointer>
#include <QtCore/QSocketNotifier>
#include <sys/epoll.h>
#include <errno.h>
#include "eventdispatcher_epoll_p.h"

void EventDispatcherEPollPrivate::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier != 0);
    Q_ASSUME(notifier != 0);

    int events = 0;
    int fd = static_cast<int>(notifier->socket());

    epoll_event e;

    SocketNotifierInfo *data;
    auto it = m_handles.find(fd);
    if (it == m_handles.end()) {
        data = new SocketNotifierInfo(fd);
        e.data.ptr = data;

        switch (notifier->type()) {
        case QSocketNotifier::Read:
            events = EPOLLIN;
            data->r = notifier;
            break;
        case QSocketNotifier::Write:
            events = EPOLLOUT;
            data->w = notifier;
            break;
        case QSocketNotifier::Exception:
            events = EPOLLPRI;
            data->x = notifier;
            break;
        default:
            Q_UNREACHABLE();
        }

        data->events = events;
        e.events = events;

        int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &e);
        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
            delete data;
            return;
        }

        m_handles.insert(fd, data);
    } else {
        data = static_cast<SocketNotifierInfo *>(it.value());
        Q_ASSERT(data);

        QSocketNotifier **n = nullptr;
        if (data) {
            e.data.ptr = data;
            switch (notifier->type()) {
            case QSocketNotifier::Read:
                events = EPOLLIN;
                n = &data->r;
                break;
            case QSocketNotifier::Write:
                events = EPOLLOUT;
                n = &data->w;
                break;
            case QSocketNotifier::Exception:
                events = EPOLLPRI;
                n = &data->x;
                break;
            default:
                Q_UNREACHABLE();
            }

            Q_ASSERT(n != 0);
            if (Q_UNLIKELY(*n != 0)) {
                qWarning("%s: cannot add two socket notifiers of the same type for the same descriptor", Q_FUNC_INFO);
                return;
            }

            Q_ASSERT((data->events & events) == 0);

            data->events |= events;
            e.events      = data->events;
            *n            = notifier;

            int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
            if (Q_UNLIKELY(res != 0)) {
                qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
                return;
            }
            data->ref(); //we are reusing data
        }
        else {
            Q_UNREACHABLE();
        }
    }

    Q_ASSERT(!m_notifiers.contains(notifier));
    m_notifiers.insert(notifier, data);
}

void EventDispatcherEPollPrivate::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier != 0);
    Q_ASSUME(notifier != 0);

    auto it = m_notifiers.find(notifier);
    if (Q_LIKELY(it != m_notifiers.end())) {
        SocketNotifierInfo *info = it.value();

        struct epoll_event e;
        e.data.ptr = info;

        if (info->r == notifier) {
            info->events &= ~EPOLLIN;
            info->r       = nullptr;
        }
        else if (info->w == notifier) {
            info->events &= ~EPOLLOUT;
            info->w       = nullptr;
        }
        else if (info->x == notifier) {
            info->events &= ~EPOLLPRI;
            info->x       = nullptr;
        }
        else {
            qFatal("%s: internal error: cannot find socket notifier", Q_FUNC_INFO);
        }

        e.events = info->events;

        int res;

        if (info->r || info->w || info->x) {
            res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, info->fd, &e);
        } else {
            res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, info->fd, &e);
            if (Q_UNLIKELY(res != 0 && EBADF == errno)) {
                res = 0;
            }

            auto hi = m_handles.find(info->fd);
            Q_ASSERT(hi != m_handles.end());
            m_handles.erase(hi);
        }

        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
        }

        m_notifiers.erase(it); // Hash is not rehashed
        info->deref();
    }
}

bool EventDispatcherEPollPrivate::disableSocketNotifiers(bool disable)
{
    epoll_event e;

    auto it = m_notifiers.constBegin();
    while (it != m_notifiers.constEnd()) {
        SocketNotifierInfo *info  = it.value();

        e.events = disable ? 0 : info->events;
        e.data.ptr = info;
        int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, info->fd, &e);
        if (Q_UNLIKELY(res != 0)) {
            qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
        }

        ++it;
    }

    return true;
}
