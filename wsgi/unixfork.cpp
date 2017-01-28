/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "unixfork.h"

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>

#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTimer>
#include <QDebug>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

static int signalsFd[2];

UnixFork::UnixFork(QObject *parent) : QObject(parent)
{
    setupUnixSignalHandlers();
}

UnixFork::~UnixFork()
{

}

void UnixFork::createProcess(int process)
{
    for (int i = 0; i < process; ++i) {
        if (!createChild()) {
            return;
        }
    }

    qDebug() << "Created workers" << process;
}

void UnixFork::killChild()
{
    const auto childs = m_childs;
    for (qint64 pid : childs) {
        if (pid) {
            ::kill(pid_t(pid), SIGKILL);
        }
    }
}

void UnixFork::terminateChild()
{
    const auto childs = m_childs;
    for (qint64 pid : childs) {
        if (pid) {
            ::kill(pid_t(pid), SIGQUIT);
        }
    }
}

void UnixFork::signalHandler(int signal)
{
//    qDebug() << Q_FUNC_INFO << signal;
    ::write(signalsFd[0], &signal, sizeof(signal));
}

void UnixFork::setGid(const QString &gid)
{
    bool ok;
    int gidInt = gid.toInt(&ok);
    if (!ok) {
        struct group *ugroup = getgrnam(gid.toUtf8().constData());
        if (ugroup) {
            gidInt = ugroup->gr_gid;
        } else {
            qFatal("setgid group %s not found.", gid.toUtf8().constData());
        }
    }

    if (setgid(gidInt)) {
        qFatal("Failed to set gid '%s'", strerror(errno));
    }
}

void UnixFork::setUid(const QString &uid)
{
    bool ok;
    int uidInt = uid.toInt(&ok);
    if (!ok) {
        struct passwd *upasswd = getpwnam(uid.toUtf8().constData());
        if (upasswd) {
            uidInt = upasswd->pw_uid;
        } else {
            qFatal("setuid user %s not found.", uid.toUtf8().constData());
        }
    }

    if (setuid(uidInt)) {
        qFatal("Failed to set uid: '%s'", strerror(errno));
    }
}

void UnixFork::chownSocket(const QString &filename, const QString &uidGid)
{
    uid_t new_uid = -1;
    uid_t new_gid = -1;
    struct group *new_group = NULL;
    struct passwd *new_user = NULL;

    const QString owner = uidGid.section(QLatin1Char(':'), 0, 0);

    bool ok;
    new_uid = owner.toInt(&ok);

    if (!ok) {
        new_user = getpwnam(owner.toUtf8().constData());
        if (!new_user) {
            qFatal("unable to find user '%s'", owner.toUtf8().constData());
        }
        new_uid = new_user->pw_uid;
    }

    const QString group = uidGid.section(QLatin1Char(':'), 1, 1);
    if (!group.isEmpty()) {
        new_gid = group.toInt(&ok);
        if (!ok) {
            new_group = getgrnam(group.toUtf8().constData());
            if (!new_group) {
                qFatal("unable to find group '%s'", group.toUtf8().constData());
            }
            new_gid = new_group->gr_gid;
        }
    }

    if (chown(filename.toUtf8().constData(), new_uid, new_gid)) {
        qFatal("chown() error '%s'", strerror(errno));
    }
}

void UnixFork::handleSigHup()
{
    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    m_proc->kill();
}

void UnixFork::handleSigTerm()
{
    // do Qt stuff
//    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    qApp->quit();
//    m_proc->terminate();
}

void UnixFork::handleSigInt()
{
    // do Qt stuff
//    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
    m_terminating = true;
    if (m_child) {
        qApp->quit();
    } else {
        std::cout << "SIGINT/SIGQUIT received, killing workers..." << std::endl;
        QTimer::singleShot(10 * 1000, [this]() {
            std::cout << "workers killing timeout, KILL ..." << std::endl;
            killChild();
            QTimer::singleShot(10 * 1000, qApp, &QCoreApplication::quit);
        });
//        terminateChild();
    }
}

void UnixFork::handleSigChld()
{
    pid_t p;
    int status;

    while ((p = waitpid(-1, &status, WNOHANG)) > 0)
    {
        /* Handle the death of pid p */
        qDebug() << Q_FUNC_INFO << "worker died" << p << status;
        // SIGTERM is used when CHEAPED (ie post fork failed)
        if (m_childs.removeOne(p) && !m_terminating && status != SIGTERM) {
            createChild();
        } else if (!m_child && m_childs.isEmpty()) {
            qApp->quit();
        }
    }
}

int UnixFork::setupUnixSignalHandlers()
{
    setupSocketPair(false);

//    struct sigaction hup;
//    hup.sa_handler = UnixFork::signalHandler;
//    sigemptyset(&hup.sa_mask);
//    hup.sa_flags = 0;
//    hup.sa_flags |= SA_RESTART;

//    if (sigaction(SIGHUP, &hup, 0) > 0)
//        return 1;

//    struct sigaction term;
//    term.sa_handler = UnixFork::signalHandler;
//    sigemptyset(&term.sa_mask);
//    term.sa_flags |= SA_RESTART;

//    if (sigaction(SIGTERM, &term, 0) > 0)
//        return 2;

//    struct sigaction kill;
//    kill.sa_handler = UnixFork::signalHandler;
//    sigemptyset(&kill.sa_mask);
//    kill.sa_flags |= SA_RESTART;

//    if (sigaction(SIGKILL, &kill, 0) > 0)
//        return 3;

    struct sigaction action;

    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGINT, &action, 0) > 0)
        return SIGINT;

    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGQUIT, &action, 0) > 0)
        return SIGQUIT;

//    struct sigaction chld;
    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;

    if (sigaction(SIGCHLD, &action, 0) > 0)
        return SIGCHLD;

    return 0;
}

void UnixFork::setupSocketPair(bool closeSignalsFD)
{
    if (closeSignalsFD) {
        close(signalsFd[0]);
        close(signalsFd[1]);
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, signalsFd)) {
        qFatal("Couldn't create SIGNALS socketpair");
    }
    delete m_signalNotifier;
    m_signalNotifier = new QSocketNotifier(signalsFd[1], QSocketNotifier::Read, this);
    connect(m_signalNotifier, &QSocketNotifier::activated, this, [this](int fd) {
        int signal;
        ::read(fd, &signal, sizeof(signal));

//        qDebug() << "Got signal:" << signal << "pid:" << QCoreApplication::applicationPid();
        switch (signal) {
        case SIGCHLD:
            handleSigChld();
            break;
        case SIGINT:
        case SIGQUIT:
            handleSigInt();
            break;
        default:
            break;
        }

        // do Qt stuff
    //    m_proc->kill();
    });
}

bool UnixFork::createChild()
{
    if (m_child) {
        return false;
    }

    qint64 childPID = fork();

    if(childPID >= 0) {
        if(childPID == 0) {
            qDebug() << "spawned uWSGI worker 2 (pid: 31576, cores: 1)";
            setupSocketPair(true);

            m_child = true;
            Q_EMIT forked();
        } else {
            m_childs.push_back(childPID);
            return true;
        }
    } else {
        qFatal("Fork failed, quitting!!!!!!");
    }

    return false;
}

#include "moc_unixfork.cpp"
