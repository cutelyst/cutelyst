#include "uwsgiprocess.h"

#include <QCoreApplication>
#include <QMimeDatabase>
#include <QSocketNotifier>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QDebug>

#ifdef Q_OS_UNIX
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

static int sighupFd[2];
static int sigtermFd[2];
static int sigkillFd[2];
static int sigintFd[2];

uwsgiProcess::uwsgiProcess(QObject *parent) : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd)) {
        qFatal("Couldn't create HUP socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
        qFatal("Couldn't create TERM socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigkillFd)) {
        qFatal("Couldn't create KILL socketpair");
    }

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd)) {
        qFatal("Couldn't create INT socketpair");
    }

    QSocketNotifier *socket;
    socket = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &uwsgiProcess::handleSigHup);

    socket = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &uwsgiProcess::handleSigTerm);

    socket = new QSocketNotifier(sigkillFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &uwsgiProcess::handleSigKill);

    socket = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(socket, &QSocketNotifier::activated, this, &uwsgiProcess::handleSigInt);

    m_proc = new QProcess(this);
    connect(m_proc, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
//    m_proc->setInputChannelMode(QProcess::ForwardedInputChannel);
    m_proc->setProcessChannelMode(QProcess::ForwardedChannels);
}

void uwsgiProcess::hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void uwsgiProcess::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void uwsgiProcess::killSignalHandler(int unused)
{
    char a = 1;
    ::write(sigkillFd[0], &a, sizeof(a));
}

void uwsgiProcess::intSignalHandler(int unused)
{
    char a = 1;
    ::write(sigintFd[0], &a, sizeof(a));
}
#endif

bool uwsgiProcess::run(const QString &appFilename, int port, bool restart)
{
    QDir projectDir;
    if (!uwsgiProcess::findProjectDir(QDir::current(), &projectDir)) {
        qDebug() << "Error: failed to find project";
        return false;
    }

    QString localFilename = appFilename;
    if (localFilename.isEmpty()) {
        localFilename = findApplication(projectDir);
    }

    QFileInfo fileInfo(localFilename);
    if (!fileInfo.exists()) {
        qDebug() << "Error: Application file not found";
        return false;
    }

    QStringList args;
    args.append(QStringLiteral("--http-socket"));
    args.append(QLatin1String(":") % QString::number(port));

    args.append(QStringLiteral("--chdir"));
    args.append(projectDir.absolutePath());

    args.append(QStringLiteral("-M"));

    args.append(QStringLiteral("--plugin"));
    args.append(QStringLiteral("cutelyst"));

    args.append(QStringLiteral("--cutelyst-app"));
    args.append(localFilename);

    if (restart) {
        args.append(QStringLiteral("--lazy"));
        args.append(QStringLiteral("--touch-reload"));
        args.append(localFilename);
    }

    if (setup_unix_signal_handlers()) {
        return false;
    }

    qDebug() << "Running: uwsgi" << args.join(QStringLiteral(" ")).toLatin1().data();

    m_proc->start(QStringLiteral("uwsgi"), args);

    return true;
}

#ifdef Q_OS_UNIX
int uwsgiProcess::setup_unix_signal_handlers()
{
    struct sigaction hup, term, inta, kill;

    hup.sa_handler = uwsgiProcess::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, 0) > 0)
        return 1;

    term.sa_handler = uwsgiProcess::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) > 0)
        return 2;

    kill.sa_handler = uwsgiProcess::killSignalHandler;
    sigemptyset(&kill.sa_mask);
    kill.sa_flags |= SA_RESTART;

    if (sigaction(SIGKILL, &kill, 0) > 0)
        return 3;

    inta.sa_handler = uwsgiProcess::intSignalHandler;
    sigemptyset(&inta.sa_mask);
    inta.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &inta, 0) > 0)
        return 4;

    return 0;
}
#endif

bool uwsgiProcess::findProjectDir(const QDir &dir, QDir *projectDir)
{
    QFile cmake(dir.absoluteFilePath(QStringLiteral("CMakeLists.txt")));
    if (cmake.exists()) {
        if (cmake.open(QFile::ReadOnly | QFile::Text)) {
            while (!cmake.atEnd()) {
                QByteArray line = cmake.readLine();
                if (line.toLower().startsWith(QByteArrayLiteral("project"))) {
                    *projectDir = dir;
                    return true;
                }
            }
        }
    }

    QDir localDir = dir;
    if (localDir.cdUp()) {
        return findProjectDir(localDir, projectDir);
    }
    return false;
}

QString uwsgiProcess::findApplication(const QDir &projectDir)
{
    QMimeDatabase m_db;

    QDirIterator it(projectDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString file = it.next();
        QMimeType mime = m_db.mimeTypeForFile(file);
        if (mime.inherits(QStringLiteral("application/x-sharedlib"))) {
            return file;
        }
    }
    return QString();
}

#ifdef Q_OS_UNIX
void uwsgiProcess::handleSigHup()
{
    QSocketNotifier *socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sighupFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO;
//    m_proc->kill();

    socket->setEnabled(true);
}

void uwsgiProcess::handleSigTerm()
{
    QSocketNotifier *socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO;
    m_proc->terminate();

    socket->setEnabled(true);
}

void uwsgiProcess::handleSigKill()
{
    QSocketNotifier *socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigkillFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO;
//    m_proc->terminate();

    socket->setEnabled(true);
}

void uwsgiProcess::handleSigInt()
{
    QSocketNotifier *socket = qobject_cast<QSocketNotifier*>(sender());
    socket->setEnabled(false);
    char tmp;
    ::read(sigintFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << Q_FUNC_INFO;
//    m_proc->terminate();

    socket->setEnabled(true);
}
#endif

void uwsgiProcess::processFinished(int exitCode)
{
    QCoreApplication::exit(exitCode);
}
