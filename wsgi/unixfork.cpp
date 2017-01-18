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

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QDebug>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

static int sighupFd[2];
static int sigtermFd[2];
static int sigkillFd[2];
static int sigintFd[2];
static int sigChldFd[2];

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

void UnixFork::hupSignalHandler(int unused)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void UnixFork::termSignalHandler(int unused)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void UnixFork::killSignalHandler(int unused)
{
    char a = 1;
    ::write(sigkillFd[0], &a, sizeof(a));
}

void UnixFork::intSignalHandler(int unused)
{
    char a = 1;
    ::write(sigintFd[0], &a, sizeof(a));
}

void UnixFork::chldSignalHandler(int unused)
{
    char a = 1;
    ::write(sigChldFd[0], &a, sizeof(a));
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
            qFatal("group %s not found.", gid.toUtf8().constData());
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
            qFatal("user %s not found.", uid.toUtf8().constData());
        }
    }

    if (setuid(uidInt)) {
        qFatal("Failed to set uid: '%s'", strerror(errno));
    }
}

void UnixFork::handleSigHup()
{
    auto socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sighupFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    m_proc->kill();

    socket->setEnabled(true);
}

void UnixFork::handleSigTerm()
{
    auto socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    qApp->quit();
//    m_proc->terminate();

    socket->setEnabled(true);
}

void UnixFork::handleSigKill()
{
    auto socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigkillFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    m_proc->terminate();

    socket->setEnabled(true);
}

void UnixFork::handleSigInt()
{
    auto socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigintFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
//    qApp->quit();

    socket->setEnabled(true);
}

void UnixFork::handleSigChld()
{
    auto socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigChldFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
    pid_t p;
    int status;

    while ((p = waitpid(-1, &status, WNOHANG)) > 0)
    {
        /* Handle the death of pid p */
        qDebug() << Q_FUNC_INFO << "died" << p << status;
        if (m_childs.removeOne(p)) {
            createChild();
        }
    }

    socket->setEnabled(true);
}

int UnixFork::setupUnixSignalHandlers()
{
    QSocketNotifier *socket;

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd)) {
        qFatal("Couldn't create HUP socketpair");
    }
    socket = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &UnixFork::handleSigHup);

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
        qFatal("Couldn't create TERM socketpair");
    }
    socket = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &UnixFork::handleSigTerm);

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigkillFd)) {
        qFatal("Couldn't create KILL socketpair");
    }
    socket = new QSocketNotifier(sigkillFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &UnixFork::handleSigKill);

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd)) {
        qFatal("Couldn't create INT socketpair");
    }
    socket = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &UnixFork::handleSigInt);

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigChldFd)) {
        qFatal("Couldn't create CHLD socketpair");
    }
    socket = new QSocketNotifier(sigChldFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &UnixFork::handleSigChld);

//    struct sigaction hup;
//    hup.sa_handler = UnixFork::hupSignalHandler;
//    sigemptyset(&hup.sa_mask);
//    hup.sa_flags = 0;
//    hup.sa_flags |= SA_RESTART;

//    if (sigaction(SIGHUP, &hup, 0) > 0)
//        return 1;

//    struct sigaction term;
//    term.sa_handler = UnixFork::termSignalHandler;
//    sigemptyset(&term.sa_mask);
//    term.sa_flags |= SA_RESTART;

//    if (sigaction(SIGTERM, &term, 0) > 0)
//        return 2;

    struct sigaction kill;
    kill.sa_handler = UnixFork::killSignalHandler;
    sigemptyset(&kill.sa_mask);
    kill.sa_flags |= SA_RESTART;

    if (sigaction(SIGKILL, &kill, 0) > 0)
        return 3;

//    struct sigaction inta;
//    inta.sa_handler = UnixFork::intSignalHandler;
//    sigemptyset(&inta.sa_mask);
//    inta.sa_flags |= SA_RESTART;

//    if (sigaction(SIGINT, &inta, 0) > 0)
//        return 4;

    struct sigaction chld;
    chld.sa_handler = UnixFork::chldSignalHandler;
    sigemptyset(&chld.sa_mask);
    chld.sa_flags |= SA_RESTART;

    if (sigaction(SIGCHLD, &chld, 0) > 0)
        return 5;

    return 0;
}

bool UnixFork::createChild()
{
    if (m_child) {
        return false;
    }

    qint64 childPID = fork();

    if(childPID >= 0) {
        if(childPID == 0) {
            m_child = true;
            Q_EMIT forked();
            deleteLater();
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
