/*
 * SPDX-FileCopyrightText: (C) 2014-2020 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "unixfork.h"

#include "server.h"

#if defined(HAS_EventLoopEPoll)
#    include "EventLoopEPoll/eventdispatcher_epoll.h"
#endif

#if defined(__FreeBSD__) || defined(__GNU_kFreeBSD__)
#    include <sys/cpuset.h>
#    include <sys/param.h>
#endif

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QFile>
#include <QLoggingCategory>
#include <QMutex>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>

Q_LOGGING_CATEGORY(C_SERVER_UNIX, "cutelyst.server.unix", QtWarningMsg)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

using namespace Qt::StringLiterals;

namespace {
int signalsFd[2];
} // namespace

UnixFork::UnixFork(int process, int threads, bool setupSignals, QObject *parent)
    : AbstractFork(parent)
    , m_threads(threads)
    , m_processes(process)
{
    if (setupSignals) {
        setupUnixSignalHandlers();
    }
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
    return true;
}

int UnixFork::exec(bool lazy, bool master)
{
    if (master) {
        std::cout << "spawned SERVER master process (pid: " << QCoreApplication::applicationPid()
                  << ")" << '\n';
    }

    int ret;
    if (lazy) {
        if (master) {
            ret = internalExec();
        } else {
            std::cerr << "*** Master mode must be set on lazy mode" << '\n';
            ret = -1;
        }
    } else {
        if (m_processes > 0) {
            ret = internalExec();
        } else {
            Q_EMIT forked(0);
            ret = qApp->exec();
        }
    }

    return ret;
}

void UnixFork::restart()
{
    for (const auto &[key, value] : m_childs.asKeyValueRange()) {
        value.restart = 1; // Mark as requiring restart
        terminateChild(key);
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
        m_recreateWorker.removeIf([this, respawn](Worker worker) {
            worker.restart = 0;
            if (!createChild(worker, respawn)) {
                std::cout << "CHEAPING worker: " << worker.id << '\n';
                --m_processes;
            }

            return true; // Clean recreate worker list
        });
    } else {
        for (int i = 0; i < m_processes; ++i) {
            Worker worker;
            worker.id   = i + 1;
            worker.null = false;
            createChild(worker, respawn);
        }
    }

    return !m_childs.empty();
}

void UnixFork::decreaseWorkerRespawn()
{
    int missingRespawn = 0;
    for (const auto &[key, value] : m_childs.asKeyValueRange()) {
        if (value.respawn > 0) {
            --value.respawn;
            missingRespawn += value.respawn;
        }
    }

    if (missingRespawn) {
        QTimer::singleShot(std::chrono::seconds{1}, this, &UnixFork::decreaseWorkerRespawn);
    }
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
    //    qCDebug(C_SERVER_UNIX) << "SIGKILL " << pid;
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
    //    qCDebug(C_SERVER_UNIX) << "SIGQUIT " << pid;
    ::kill(pid_t(pid), SIGQUIT);
}

void UnixFork::stopSERVER(const QString &pidfile)
{
    QFile file(pidfile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Failed open pid file " << qPrintable(pidfile) << '\n';
        exit(1);
    }

    QByteArray piddata = file.readLine().simplified();
    qint64 pid         = piddata.toLongLong();
    if (pid < 2) {
        std::cerr << "Failed read pid file " << qPrintable(pidfile) << '\n';
        exit(1);
    }

    ::kill(pid_t(pid), SIGINT);
    exit(0);
}

bool UnixFork::setUmask(const QByteArray &valueStr)
{
    if (valueStr.size() < 3) {
        std::cerr << "umask too small" << '\n';
        return false;
    }

    const char *value = valueStr.constData();
    mode_t mode       = 0;
    if (valueStr.size() == 3) {
        mode = (mode << 3) + (value[0] - '0');
        mode = (mode << 3) + (value[1] - '0');
        mode = (mode << 3) + (value[2] - '0');
    } else {
        mode = (mode << 3) + (value[1] - '0');
        mode = (mode << 3) + (value[2] - '0');
        mode = (mode << 3) + (value[3] - '0');
    }
    std::cout << "umask() " << value << '\n';

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
        m_checkChildRestart->start(std::chrono::milliseconds{500});
        connect(m_checkChildRestart, &QTimer::timeout, this, &UnixFork::handleSigChld);
    }
}

void UnixFork::postFork(int workerId)
{
    // Child must not have parent timers
    delete m_checkChildRestart;

    Q_EMIT forked(workerId - 1);
}

bool UnixFork::setGidUid(const QString &gid, const QString &uid, bool noInitgroups)
{
    bool ok;

    if (!gid.isEmpty()) {
        uint gidInt = gid.toUInt(&ok);
        if (!ok) {
            const struct group *ugroup = getgrnam(qUtf8Printable(gid));
            if (ugroup) {
                gidInt = ugroup->gr_gid;
            } else {
                std::cerr << "setgid group %s not found." << qUtf8Printable(gid) << '\n';
                return false;
            }
        }

        if (setgid(gidInt)) {
            std::cerr << "Failed to set gid '%s'" << strerror(errno) << '\n';
            return false;
        }
        std::cout << "setgid() to " << gidInt << '\n';

        if (noInitgroups || uid.isEmpty()) {
            if (setgroups(0, nullptr)) {
                std::cerr << "Failed to setgroups()" << '\n';
                return false;
            }
        } else {
            QByteArray uidname;
            uint uidInt = uid.toUInt(&ok);
            if (ok) {
                const struct passwd *pw = getpwuid(uidInt);
                if (pw) {
                    uidname = pw->pw_name;
                }
            } else {
                uidname = uid.toUtf8();
            }

            if (initgroups(uidname.constData(), gidInt)) {
                std::cerr << "Failed to setgroups()" << '\n';
                return false;
            }
        }
    }

    if (!uid.isEmpty()) {
        uint uidInt = uid.toUInt(&ok);
        if (!ok) {
            const struct passwd *upasswd = getpwnam(qUtf8Printable(uid));
            if (upasswd) {
                uidInt = upasswd->pw_uid;
            } else {
                std::cerr << "setuid user" << qUtf8Printable(uid) << "not found." << '\n';
                return false;
            }
        }

        if (setuid(uidInt)) {
            std::cerr << "Failed to set uid:" << strerror(errno) << '\n';
            return false;
        }
        std::cout << "setuid() to " << uidInt << '\n';
    }
    return true;
}

void UnixFork::chownSocket(const QString &filename, const QString &uidGid)
{
    const QString owner = uidGid.section(u':', 0, 0);

    bool ok;
    uid_t new_uid = owner.toUInt(&ok);

    if (!ok) {
        const struct passwd *new_user = getpwnam(qUtf8Printable(owner));
        if (!new_user) {
            qFatal("unable to find user '%s'", qUtf8Printable(owner));
        }
        new_uid = new_user->pw_uid;
    }

    gid_t new_gid       = -1U;
    const QString group = uidGid.section(u':', 1, 1);
    if (!group.isEmpty()) {
        new_gid = group.toUInt(&ok);
        if (!ok) {
            const struct group *new_group = getgrnam(qUtf8Printable(group));
            if (!new_group) {
                qFatal("unable to find group '%s'", qUtf8Printable(group));
            }
            new_gid = new_group->gr_gid;
        }
    }

    if (chown(qUtf8Printable(filename), new_uid, new_gid)) {
        qFatal("chown() error '%s'", strerror(errno));
    }
}

#ifdef Q_OS_LINUX
// static int cpuSockets = -1;

// socket/cores
int parseProcCpuinfo()
{
    int cpuSockets = 1;
    //    std::pair<int, int> ret;

    //    static QMutex mutex;
    //    QMutexLocker locker(&mutex);
    QFile file(u"/proc/cpuinfo"_s);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(C_SERVER_UNIX) << "Failed to open file" << file.errorString();
        //        cpuSockets = 1;
        //        cpuCores = QThread::idealThreadCount();
        return cpuSockets;
    }

    char buf[1024];
    qint64 lineLength;
    QByteArrayList physicalIds;
    //    cpuCores = 0;
    while ((lineLength = file.readLine(buf, sizeof(buf))) != -1) {
        const QByteArray line(buf, int(lineLength));
        if (line.startsWith("physical id\t: ")) {
            const QByteArray id = line.mid(14).trimmed();
            if (!physicalIds.contains(id)) {
                physicalIds.push_back(id);
            }
        } /* else if (line.startsWith("processor \t: ")) {
             ++cpuCores;
         }*/
    }

    //    if (cpuCores == 0) {
    //        cpuCores = QThread::idealThreadCount();
    //    }

    if (!physicalIds.isEmpty()) {
        cpuSockets = physicalIds.size();
    } else {
        cpuSockets = 1;
    }
    return cpuSockets;
}
#endif

int UnixFork::idealProcessCount()
{
#ifdef Q_OS_LINUX
    static int cpuSockets = parseProcCpuinfo();

    return cpuSockets;
#else
    return 1;
#endif
}

int UnixFork::idealThreadCount()
{
#ifdef Q_OS_LINUX
    static int cpuCores = qMax(1, QThread::idealThreadCount());

    return cpuCores;
#else
    return qMax(1, QThread::idealThreadCount());
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
        qDebug(C_SERVER_UNIX) << "SIGINT/SIGQUIT received, worker shutting down...";
        Q_EMIT shutdown();
    } else {
        std::cout << "SIGINT/SIGQUIT received, terminating workers..." << '\n';
        setupCheckChildTimer();

        static int count = 0;
        if (count++ > 2) {
            std::cout << "KILL workers..." << '\n';
            killChild();
            QTimer::singleShot(std::chrono::seconds{3}, qApp, &QCoreApplication::quit);
        } else if (count > 1) {
            terminateChild();
        } else {
            QTimer::singleShot(std::chrono::seconds{30}, this, [this]() {
                std::cout << "workers terminating timeout, KILL ..." << '\n';
                killChild();
                QTimer::singleShot(std::chrono::seconds{30}, qApp, &QCoreApplication::quit);
            });

            terminateChild();
        }
    }
}

void UnixFork::handleSigChld()
{
    pid_t p;
    int status;

    while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
        /* Handle the death of pid p */
        //        qCDebug(C_SERVER_UNIX) << "SIGCHLD worker died" << p << WEXITSTATUS(status);
        // SIGTERM is used when CHEAPED (ie post fork failed)
        int exitStatus = WEXITSTATUS(status);

        Worker worker;
        auto it = m_childs.constFind(p);
        if (it != m_childs.constEnd()) {
            worker = it.value();
            m_childs.erase(it);
        } else {
            std::cout << "DAMN ! *UNKNOWN* worker (pid: " << p << ") died, killed by signal "
                      << exitStatus << " :( ignoring .." << '\n';
            continue;
        }

        if (WIFEXITED(status) && exitStatus == 15 && worker.restart == 0) {
            // Child process cheaping
            worker.null = true;
        }

        if (!worker.null && !m_terminating) {
            if (worker.restart == 0) {
                std::cout << "DAMN ! worker " << worker.id << " (pid: " << p
                          << ") died, killed by signal " << exitStatus << " :( trying respawn .."
                          << '\n';
            }
            worker.restart = 0;
            ++worker.respawn;
            QTimer::singleShot(std::chrono::seconds{1}, this, &UnixFork::decreaseWorkerRespawn);
            m_recreateWorker.push_back(worker);
            qApp->quit();
        } else if (!m_child && m_childs.isEmpty()) {
            qApp->quit();
        }
    }

    if (m_checkChildRestart) {
        bool allRestarted = true;
        for (const auto &[key, value] : m_childs.asKeyValueRange()) {
            if (value.restart) {
                if (++value.restart > 10) {
                    killChild(key);
                }
                allRestarted = false;
            }
        }

        if (allRestarted) {
            m_checkChildRestart->deleteLater();
            m_checkChildRestart = nullptr;
        }
    }
}

void UnixFork::setSched(Cutelyst::Server *server, int workerId, int workerCore)
{
    int cpu_affinity = server->cpuAffinity();
    if (cpu_affinity) {
        char buf[4096];

        int pos =
            snprintf(buf, 4096, "mapping worker %d core %d to CPUs:", workerId + 1, workerCore + 1);
        if (pos < 25 || pos >= 4096) {
            qCCritical(C_SERVER_UNIX) << "unable to initialize cpu affinity !!!";
            exit(1);
        }
#if defined(__linux__) || defined(__GNU_kFreeBSD__)
        cpu_set_t cpuset;
#elif defined(__FreeBSD__)
        cpuset_t cpuset;
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__GNU_kFreeBSD__)
        int coreCount = idealThreadCount();

        int workerThreads = 1;
        if (server->threads().compare(u"auto") == 0) {
            workerThreads = coreCount;
        } else if (server->threads().toInt() > 1) {
            workerThreads = server->threads().toInt();
        }

        int base_cpu;
        if (workerThreads > 1) {
            base_cpu = (workerId * workerThreads) + workerCore * cpu_affinity;
        } else {
            base_cpu = workerId * cpu_affinity;
        }

        if (base_cpu >= coreCount) {
            base_cpu = base_cpu % coreCount;
        }

        CPU_ZERO(&cpuset);
        for (int i = 0; i < cpu_affinity; i++) {
            if (base_cpu >= coreCount) {
                base_cpu = 0;
            }
            CPU_SET(base_cpu, &cpuset);
            int ret = snprintf(buf + pos, 4096 - pos, " %d", base_cpu + 1);
            if (ret < 2 || ret >= 4096) {
                qCCritical(C_SERVER_UNIX) << "unable to initialize cpu affinity !!!";
                exit(1);
            }
            pos += ret;
            base_cpu++;
        }
#endif
#if defined(__linux__) || defined(__GNU_kFreeBSD__)
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset)) {
            qFatal("failed to sched_setaffinity()");
        }
#elif defined(__FreeBSD__)
        if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, -1, sizeof(cpuset), &cpuset)) {
            qFatal("cpuset_setaffinity");
        }
#endif
        std::cout << buf << '\n';
    }
}

int UnixFork::setupUnixSignalHandlers()
{
    setupSocketPair(false, true);

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
    if (sigaction(SIGINT, &action, nullptr) > 0) {
        return SIGINT;
    }

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGQUIT, &action, nullptr) > 0) {
        return SIGQUIT;
    }

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = UnixFork::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;

    if (sigaction(SIGCHLD, &action, nullptr) > 0) {
        return SIGCHLD;
    }

    return 0;
}

void UnixFork::setupSocketPair(bool closeSignalsFD, bool createPair)
{
    if (closeSignalsFD) {
        close(signalsFd[0]);
        close(signalsFd[1]);
    }

    if (createPair && ::socketpair(AF_UNIX, SOCK_STREAM, 0, signalsFd)) {
        qFatal("Couldn't create SIGNALS socketpair");
    }
    delete m_signalNotifier;

    m_signalNotifier = new QSocketNotifier(signalsFd[1], QSocketNotifier::Read, this);
    connect(m_signalNotifier, &QSocketNotifier::activated, this, [this]() {
        char signal;
        read(signalsFd[1], &signal, sizeof(signal));

        //        qCDebug(C_SERVER_UNIX) << "Got signal:" << static_cast<int>(signal) << "pid:" <<
        //        QCoreApplication::applicationPid();
        switch (signal) {
        case SIGCHLD:
            QTimer::singleShot(std::chrono::seconds{0}, this, &UnixFork::handleSigChld);
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

    delete m_signalNotifier;
    m_signalNotifier = nullptr;

    qint64 childPID = fork();

    if (childPID >= 0) {
        if (childPID == 0) {
            if (worker.respawn >= 5) {
                std::cout << "SERVER worker " << worker.id << " respawned too much, sleeping a bit"
                          << '\n';
                sleep(2);
            }

#if defined(HAS_EventLoopEPoll)
            auto epoll = qobject_cast<EventDispatcherEPoll *>(QAbstractEventDispatcher::instance());
            if (epoll) {
                epoll->reinstall();
            }
#endif

            setupSocketPair(true, true);

            m_child = true;
            postFork(worker.id);

            int ret = qApp->exec();
            _exit(ret);
        } else {
            setupSocketPair(false, false);

            if (respawn) {
                std::cout << "Respawned SERVER worker " << worker.id << " (new pid: " << childPID
                          << ", cores: " << m_threads << ")" << '\n';
            } else {
                if (m_processes == 1) {
                    std::cout << "spawned SERVER worker (and the only) (pid: " << childPID
                              << ", cores: " << m_threads << ")" << '\n';
                } else {
                    std::cout << "spawned SERVER worker " << worker.id << " (pid: " << childPID
                              << ", cores: " << m_threads << ")" << '\n';
                }
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
