/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include <sys/stat.h>
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
#include <QThread>
#include <QFile>
#include <QLoggingCategory>
#include <QFileSystemWatcher>

Q_LOGGING_CATEGORY(WSGI_UNIX, "wsgi.unix")

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

static int signalsFd[2];

UnixFork::UnixFork(int process, int threads, QObject *parent) : AbstractFork(parent)
  , m_threads(threads)
  , m_processes(process)
{
    setupUnixSignalHandlers();
}

UnixFork::~UnixFork()
{
    if (m_child) {
        _exit(0);
    }
}

bool UnixFork::continueMaster(int *exit)
{
    Q_UNUSED(exit)
    if (m_processes == 0) {
        m_processes = 1;
    }
    return true;
}

int UnixFork::exec(bool lazy, bool master)
{
    int ret;
    if (lazy) {
        if (master) {
            connect(this, &UnixFork::forked, this, &UnixFork::setupApplication);
            ret = internalExec();
        } else {
            std::cerr << "*** Master mode must be set on lazy mode" << std::endl;
        }
    } else {
        Q_EMIT setupApplication();

        if (m_processes) {
            // Forking with an event loop running leads to
            // non working event loops on child process
            // so this is a temporary loop until all engines
            // are initted
            qApp->exec();

            if (!m_enginedInitted) {
                std::cerr << "Failed to init engines" << std::endl;
                return 1;
            }

            ret = internalExec();
        } else {
            ret = qApp->exec();
        }
    }

    return ret;
}

void UnixFork::restart()
{
    auto it = m_childs.begin();
    while (it != m_childs.end()) {
        it.value().restart = 1; // Mark as requiring restart
        terminateChild(it.key());
        ++it;
    }

    setupCheckChildTimer();
}

int UnixFork::internalExec()
{
    int ret;
    bool respawn = false;
    do {
        if (!createProcess(respawn)) {
            return 1;
        }
        respawn = true;

        installTouchReload();

        ret = qApp->exec();

        removeTouchReload();
    } while (!m_terminating);

    return ret;
}

bool UnixFork::createProcess(bool respawn)
{
    if (respawn) {
        auto it = m_recreateWorker.begin();
        while (it != m_recreateWorker.end()) {
            Worker worker = *it;
            worker.restart = 0;
            if (!createChild(worker, respawn)) {
                std::cout << "CHEAPING worker: " << worker.id << std::endl;
                --m_processes;
            }
            m_recreateWorker.erase(it);
        }
    } else {
        for (int i = 0; i < m_processes; ++i) {
            Worker worker;
            worker.id = i + 1;
            worker.null = false;
            createChild(worker, respawn);
        }
    }

    return !m_childs.empty();
}

void UnixFork::killChild()
{
    const auto childs = m_childs.keys();
    for (qint64 pid : childs) {
        killChild(pid);
    }
}

void UnixFork::killChild(qint64 pid)
{
//    qCDebug(WSGI_UNIX) << "SIGKILL " << pid;
    ::kill(pid_t(pid), SIGKILL);
}

void UnixFork::terminateChild()
{
    const auto childs = m_childs.keys();
    for (qint64 pid : childs) {
        terminateChild(pid);
    }
}

void UnixFork::terminateChild(qint64 pid)
{
//    qCDebug(WSGI_UNIX) << "SIGQUIT " << pid;
    ::kill(pid_t(pid), SIGQUIT);
}

void UnixFork::enginesInitted()
{
    m_enginedInitted = true;
}

void UnixFork::stopWSGI(const QString &pidfile)
{
    QFile file(pidfile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Failed open pid file " << pidfile.toLocal8Bit().constData() << std::endl;
        exit(1);
    }

    QByteArray piddata = file.readLine().simplified();
    qint64 pid = piddata.toLongLong();
    if (pid < 2) {
        std::cerr << "Failed read pid file " << pidfile.toLocal8Bit().constData() << std::endl;
        exit(1);
    }

    ::kill(pid_t(pid), SIGINT);
    exit(0);
}

bool UnixFork::setUmask(const QString &valueStr)
{
    if (valueStr.size() < 3) {
        std::cerr << "umask too small" << std::endl;
        return false;
    }

    const char *value = valueStr.toLatin1().constData();
    mode_t mode = 0;
    if (valueStr.size() == 3) {
        mode = (mode << 3) + (value[0] - '0');
        mode = (mode << 3) + (value[1] - '0');
        mode = (mode << 3) + (value[2] - '0');
    }
    else {
        mode = (mode << 3) + (value[1] - '0');
        mode = (mode << 3) + (value[2] - '0');
        mode = (mode << 3) + (value[3] - '0');
    }
    std::cout << "umask() " << value << std::endl;

    umask(mode);

    return true;
}

void UnixFork::signalHandler(int signal)
{
//    qDebug() << Q_FUNC_INFO << signal << QCoreApplication::applicationPid();
    char sig = signal;
    write(signalsFd[0], &sig, sizeof(sig));
}

void UnixFork::setupCheckChildTimer()
{
    if (!m_checkChildRestart) {
        m_checkChildRestart = new QTimer(this);
        m_checkChildRestart->start(500);
        connect(m_checkChildRestart, &QTimer::timeout, this, &UnixFork::handleSigChld);
    }
}

void UnixFork::postFork(int workerId)
{
    // Child must not have parent timers
    delete m_checkChildRestart;

    Q_EMIT forked(workerId);
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
    std::cout << "setgid() " << gidInt << std::endl;
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
    std::cout << "setuid() " << uidInt << std::endl;
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

static int cpuSockets = -1;
static int cpuCores = -1;

void parseProcCpuinfo() {
    QFile file(QStringLiteral("/proc/cpuinfo"));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(WSGI_UNIX) << "Failed to open file" << file.errorString();
        cpuSockets = 1;
        return;
    }

    char buf[1024];
    qint64 lineLength;
    QByteArrayList physicalIds;
    cpuCores = 0;
    while ((lineLength = file.readLine(buf, sizeof(buf))) != -1) {
        const QByteArray line(buf, lineLength);
        if (line.startsWith("physical id\t: ")) {
            const QByteArray id = line.mid(14).trimmed();
            if (!physicalIds.contains(id)) {
                physicalIds.push_back(id);
            }
        } else if (line.startsWith("processor \t: ")) {
            ++cpuCores;
        }
    }

    if (cpuCores == 0) {
        cpuCores = QThread::idealThreadCount();
    }

    if (physicalIds.size()) {
        cpuSockets = physicalIds.size();
    } else {
        cpuSockets = 1;
    }
}

int UnixFork::idealProcessCount()
{
#ifdef Q_OS_LINUX
    if (cpuSockets == -1) {
        parseProcCpuinfo();
    }

    return cpuSockets;
#else
    return 1;
#endif
}

int UnixFork::idealThreadCount()
{
#ifdef Q_OS_LINUX
    if (cpuCores == -1) {
        parseProcCpuinfo();
    }

    return cpuCores;
#else
    return QThread::idealThreadCount();
#endif
}

void UnixFork::handleSigHup()
{
    // do Qt stuff
//    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();
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
    if (m_child || (m_childs.isEmpty())) {
        Q_EMIT shutdown();
    } else {
        std::cout << "SIGINT/SIGQUIT received, killing workers..." << std::endl;
        setupCheckChildTimer();

        QTimer::singleShot(30 * 1000, [this]() {
            std::cout << "workers terminating timeout, KILL ..." << std::endl;
            killChild();
            QTimer::singleShot(3 * 1000, qApp, &QCoreApplication::quit);
        });

        terminateChild();
    }
}

void UnixFork::handleSigChld()
{
    pid_t p;
    int status;

    while ((p = waitpid(-1, &status, WNOHANG)) > 0)
    {
        /* Handle the death of pid p */
//        qCDebug(WSGI_UNIX) << "SIGCHLD worker died" << p << status;
        // SIGTERM is used when CHEAPED (ie post fork failed)
        Worker worker;
        auto it = m_childs.find(p);
        if (it != m_childs.end()) {
            worker = it.value();
            m_childs.erase(it);
        }

        int exitStatus = WEXITSTATUS(status);
        if (WIFEXITED(status) && exitStatus == 15) {
            // Child process cheaping
            worker.null = true;
        }

        if (!worker.null && !m_terminating/* && status != SIGTERM*/) {
            std::cout << "DAMN ! worker " << worker.id << " (pid: " << p << ") died, killed by signal " << exitStatus << " :( trying respawn .." << std::endl;
            m_recreateWorker.push_back(worker);
            qApp->quit();
        } else if (!m_child && m_childs.isEmpty()) {
            qApp->quit();
        }
    }

    if (m_checkChildRestart) {
        bool allRestarted = true;
        auto it = m_childs.begin();
        while (it != m_childs.end()) {
            if (it.value().restart) {
                if (++it.value().restart > 10) {
                    killChild(it.key());
                }
                allRestarted = false;
            }
            ++it;
        }

        if (allRestarted) {
            m_checkChildRestart->deleteLater();
            m_checkChildRestart = nullptr;
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

    struct sigaction action;

//    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGINT, &action, 0) > 0)
        return SIGINT;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGQUIT, &action, 0) > 0)
        return SIGQUIT;

    memset(&action, 0, sizeof(struct sigaction));
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
    connect(m_signalNotifier, &QSocketNotifier::activated, this, [this]() {
        char signal;
        read(signalsFd[1], &signal, sizeof(signal));

//        qCDebug(WSGI_UNIX) << "Got signal:" << static_cast<int>(signal) << "pid:" << QCoreApplication::applicationPid();
        switch (signal) {
        case SIGCHLD:
            QTimer::singleShot(0, this, &UnixFork::handleSigChld);
            break;
        case SIGINT:
        case SIGQUIT:
            handleSigInt();
            break;
        default:
            break;
        }
    });
}

bool UnixFork::createChild(const Worker &worker, bool respawn)
{
    if (m_child) {
        return false;
    }

    qint64 childPID = fork();

    if(childPID >= 0) {
        if(childPID == 0) {
            setupSocketPair(true);

            m_child = true;
            postFork(worker.id);

            int ret = qApp->exec();
            _exit(ret);
        } else {
            if (respawn) {
                std::cout << "Respawned WSGI worker " << worker.id << " (new pid: " << childPID << ", cores: " << m_threads << ")" << std::endl;
            } else {
                std::cout << "spawned WSGI worker " << worker.id << " (pid: " << childPID << ", cores: " << m_threads << ")" << std::endl;
            }
            m_childs.insert(childPID, worker);
            return true;
        }
    } else {
        qFatal("Fork failed, quitting!!!!!!");
    }

    return false;
}

#include "moc_unixfork.cpp"
