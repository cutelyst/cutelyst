/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "cwsgiengine.h"
#include "localserver.h"
#include "protocol.h"
#include "protocolfastcgi.h"
#include "protocolhttp.h"
#include "protocolhttp2.h"
#include "server_p.h"
#include "socket.h"
#include "tcpserverbalancer.h"

#ifdef Q_OS_UNIX
#    include "unixfork.h"
#else
#    include "windowsfork.h"
#endif

#ifdef Q_OS_LINUX
#    include "../EventLoopEPoll/eventdispatcher_epoll.h"
#    include "systemdnotify.h"
#endif

#include <iostream>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QMetaProperty>
#include <QPluginLoader>
#include <QSettings>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>
#include <QUrl>

Q_LOGGING_CATEGORY(CUTELYST_SERVER, "cutelyst.server", QtWarningMsg)

using namespace Cutelyst;

Server::Server(QObject *parent)
    : QObject(parent)
    , d_ptr(new ServerPrivate(this))
{
    QCoreApplication::addLibraryPath(QDir().absolutePath());

    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qSetMessagePattern(QStringLiteral("%{pid}:%{threadid} %{category}[%{type}] %{message}"));
    }

#ifdef Q_OS_LINUX
    if (!qEnvironmentVariableIsSet("CUTELYST_QT_EVENT_LOOP")) {
        qCInfo(CUTELYST_SERVER) << "Trying to install EPoll event loop";
        QCoreApplication::setEventDispatcher(new EventDispatcherEPoll);
    }
#endif

    auto cleanUp = [this]() {
        Q_D(Server);
        delete d->protoHTTP;
        d->protoHTTP = nullptr;

        delete d->protoHTTP2;
        d->protoHTTP2 = nullptr;

        delete d->protoFCGI;
        d->protoFCGI = nullptr;

        delete d->engine;
        d->engine = nullptr;

        qDeleteAll(d->servers);
        d->servers.clear();
    };

    connect(this, &Server::errorOccured, this, cleanUp);
    connect(this, &Server::stopped, this, cleanUp);
}

Server::~Server()
{
    delete d_ptr;
    std::cout << "Cutelyst-Server terminated" << std::endl;
}

void Server::parseCommandLine(const QStringList &arguments)
{
    Q_D(Server);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Fast, developer-friendly server"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniOpt(QStringLiteral("ini"),
                              QCoreApplication::translate("main", "load config from ini file"),
                              QCoreApplication::translate("main", "file"));
    parser.addOption(iniOpt);

    QCommandLineOption jsonOpt({QStringLiteral("j"), QStringLiteral("json")},
                               QCoreApplication::translate("main", "load config from JSON file"),
                               QCoreApplication::translate("main", "file"));
    parser.addOption(jsonOpt);

    QCommandLineOption chdir(QStringLiteral("chdir"),
                             QCoreApplication::translate("main", "chdir to specified directory before apps loading"),
                             QCoreApplication::translate("main", "directory"));
    parser.addOption(chdir);

    QCommandLineOption chdir2(QStringLiteral("chdir2"),
                              QCoreApplication::translate("main", "chdir to specified directory afterapps loading"),
                              QCoreApplication::translate("main", "directory"));
    parser.addOption(chdir2);

    QCommandLineOption lazyOption(QStringLiteral("lazy"),
                                  QCoreApplication::translate("main", "set lazy mode (load app in workers instead of master)"));
    parser.addOption(lazyOption);

    QCommandLineOption application({QStringLiteral("application"), QStringLiteral("a")},
                                   QCoreApplication::translate("main", "Application to load"),
                                   QCoreApplication::translate("main", "file"));
    parser.addOption(application);

    QCommandLineOption threads({QStringLiteral("threads"), QStringLiteral("t")},
                               QCoreApplication::translate("main", "Number of thread to use"),
                               QCoreApplication::translate("main", "threads"));
    parser.addOption(threads);

#ifdef Q_OS_UNIX
    QCommandLineOption processes({QStringLiteral("processes"), QStringLiteral("p")},
                                 QCoreApplication::translate("main", "spawn the specified number of processes"),
                                 QCoreApplication::translate("main", "processes"));
    parser.addOption(processes);
#endif

    QCommandLineOption master({QStringLiteral("master"), QStringLiteral("M")},
                              QCoreApplication::translate("main", "Enable master process"));
    parser.addOption(master);

    QCommandLineOption listenQueue({QStringLiteral("listen"), QStringLiteral("l")},
                                   QCoreApplication::translate("main", "set the socket listen queue size"),
                                   QCoreApplication::translate("main", "size"));
    parser.addOption(listenQueue);

    QCommandLineOption bufferSize({QStringLiteral("buffer-size"), QStringLiteral("b")},
                                  QCoreApplication::translate("main", "set internal buffer size"),
                                  QCoreApplication::translate("main", "bytes"));
    parser.addOption(bufferSize);

    QCommandLineOption postBuffering(QStringLiteral("post-buffering"),
                                     QCoreApplication::translate("main", "set size after which will buffer to disk instead of memory"),
                                     QCoreApplication::translate("main", "bytes"));
    parser.addOption(postBuffering);

    QCommandLineOption postBufferingBufsize(QStringLiteral("post-buffering-bufsize"),
                                            QCoreApplication::translate("main", "set buffer size for read() in post buffering mode"),
                                            QCoreApplication::translate("main", "bytes"));
    parser.addOption(postBufferingBufsize);

    QCommandLineOption httpSocketOpt({QStringLiteral("http-socket"), QStringLiteral("h1")},
                                     QCoreApplication::translate("main", "bind to the specified TCP socket using HTTP protocol"),
                                     QCoreApplication::translate("main", "address"));
    parser.addOption(httpSocketOpt);

    QCommandLineOption http2SocketOpt({QStringLiteral("http2-socket"), QStringLiteral("h2")},
                                      QCoreApplication::translate("main", "bind to the specified TCP socket using HTTP/2 protocol"),
                                      QCoreApplication::translate("main", "address"));
    parser.addOption(http2SocketOpt);

    QCommandLineOption http2HeaderTableSizeOpt(QStringLiteral("http2-header-table-size"),
                                               QCoreApplication::translate("main", "Defined the HTTP/2 header table size"),
                                               QCoreApplication::translate("main", "size"));
    parser.addOption(http2HeaderTableSizeOpt);

    QCommandLineOption upgradeH2cOpt(QStringLiteral("upgrade-h2c"),
                                     QCoreApplication::translate("main", "Upgrades HTTP/1 to H2c (HTTP/2 Clear Text)"));
    parser.addOption(upgradeH2cOpt);

    QCommandLineOption httpsH2Opt(QStringLiteral("https-h2"),
                                  QCoreApplication::translate("main", "Negotiate HTTP/2 on HTTPS socket"));
    parser.addOption(httpsH2Opt);

    QCommandLineOption httpsSocketOpt({QStringLiteral("https-socket"), QStringLiteral("hs1")},
                                      QCoreApplication::translate("main", "bind to the specified TCP socket using HTTPS protocol"),
                                      QCoreApplication::translate("main", "address"));
    parser.addOption(httpsSocketOpt);

    QCommandLineOption fastcgiSocketOpt(QStringLiteral("fastcgi-socket"),
                                        QCoreApplication::translate("main", "bind to the specified UNIX/TCP socket using FastCGI protocol"),
                                        QCoreApplication::translate("main", "address"));
    parser.addOption(fastcgiSocketOpt);

    QCommandLineOption socketAccess(QStringLiteral("socket-access"),
                                    QCoreApplication::translate("main", "set the LOCAL socket access, such as 'ugo' standing for User, Group, Other access"),
                                    QCoreApplication::translate("main", "options"));
    parser.addOption(socketAccess);

    QCommandLineOption socketTimeout({QStringLiteral("socket-timeout"), QStringLiteral("z")},
                                     QCoreApplication::translate("main", "set internal sockets timeout"),
                                     QCoreApplication::translate("main", "seconds"));
    parser.addOption(socketTimeout);

    QCommandLineOption staticMapOpt(QStringLiteral("static-map"),
                                    QCoreApplication::translate("main", "map mountpoint to static directory (or file)"),
                                    QCoreApplication::translate("main", "mountpoint=path"));
    parser.addOption(staticMapOpt);

    QCommandLineOption staticMap2Opt(QStringLiteral("static-map2"),
                                     QCoreApplication::translate("main", "like static-map but completely appending the requested resource to the docroot"),
                                     QCoreApplication::translate("main", "mountpoint=path"));
    parser.addOption(staticMap2Opt);

    QCommandLineOption autoReload({QStringLiteral("auto-restart"), QStringLiteral("r")},
                                  QCoreApplication::translate("main", "auto restarts when the application file changes"));
    parser.addOption(autoReload);

    QCommandLineOption touchReloadOpt(QStringLiteral("touch-reload"),
                                      QCoreApplication::translate("main", "reload application if the specified file is modified/touched"),
                                      QCoreApplication::translate("main", "file"));
    parser.addOption(touchReloadOpt);

    QCommandLineOption tcpNoDelay(QStringLiteral("tcp-nodelay"),
                                  QCoreApplication::translate("main", "enable TCP NODELAY on each request"));
    parser.addOption(tcpNoDelay);

    QCommandLineOption soKeepAlive(QStringLiteral("so-keepalive"),
                                   QCoreApplication::translate("main", "enable TCP KEEPALIVEs"));
    parser.addOption(soKeepAlive);

    QCommandLineOption socketSndbuf(QStringLiteral("socket-sndbuf"),
                                    QCoreApplication::translate("main", "set SO_SNDBUF"),
                                    QCoreApplication::translate("main", "bytes"));
    parser.addOption(socketSndbuf);

    QCommandLineOption socketRcvbuf(QStringLiteral("socket-rcvbuf"),
                                    QCoreApplication::translate("main", "set SO_RCVBUF"),
                                    QCoreApplication::translate("main", "bytes"));
    parser.addOption(socketRcvbuf);

    QCommandLineOption wsMaxSize(QStringLiteral("websocket-max-size"),
                                 QCoreApplication::translate("main", "sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF socket option"),
                                 QCoreApplication::translate("main", "Kbytes"));
    parser.addOption(wsMaxSize);

    QCommandLineOption pidfileOpt(QStringLiteral("pidfile"),
                                  QCoreApplication::translate("main", "create pidfile (before privileges drop)"),
                                  QCoreApplication::translate("main", "file"));
    parser.addOption(pidfileOpt);

    QCommandLineOption pidfile2Opt(QStringLiteral("pidfile2"),
                                   QCoreApplication::translate("main", "create pidfile (after privileges drop)"),
                                   QCoreApplication::translate("main", "file"));
    parser.addOption(pidfile2Opt);

#ifdef Q_OS_UNIX
    QCommandLineOption stopOption(QStringLiteral("stop"),
                                  QCoreApplication::translate("main", "stop an instance"),
                                  QCoreApplication::translate("main", "pidfile"));
    parser.addOption(stopOption);

    QCommandLineOption uidOption(QStringLiteral("uid"),
                                 QCoreApplication::translate("main", "setuid to the specified user/uid"),
                                 QCoreApplication::translate("main", "user/uid"));
    parser.addOption(uidOption);

    QCommandLineOption gidOption(QStringLiteral("gid"),
                                 QCoreApplication::translate("main", "setgid to the specified group/gid"),
                                 QCoreApplication::translate("main", "group/gid"));
    parser.addOption(gidOption);

    QCommandLineOption noInitgroupsOption(QStringLiteral("no-initgroups"),
                                          QCoreApplication::translate("main", "disable additional groups set via initgroups()"));
    parser.addOption(noInitgroupsOption);

    QCommandLineOption chownSocketOption(QStringLiteral("chown-socket"),
                                         QCoreApplication::translate("main", "chown unix sockets"),
                                         QCoreApplication::translate("main", "uid:gid"));
    parser.addOption(chownSocketOption);

    QCommandLineOption umaskOption(QStringLiteral("umask"),
                                   QCoreApplication::translate("main", "set file mode creation mask"),
                                   QCoreApplication::translate("main", "mode"));
    parser.addOption(umaskOption);

    QCommandLineOption cpuAffinityOption(QStringLiteral("cpu-affinity"),
                                         QCoreApplication::translate("main", "set CPU affinity with the number of CPUs available for each worker core"),
                                         QCoreApplication::translate("main", "core count"));
    parser.addOption(cpuAffinityOption);
#endif // Q_OS_UNIX

#ifdef Q_OS_LINUX
    QCommandLineOption reusePortOption(QStringLiteral("reuse-port"),
                                       QCoreApplication::translate("main", "enable SO_REUSEPORT flag on socket (Linux 3.9+)"));
    parser.addOption(reusePortOption);
#endif

    QCommandLineOption threadBalancerOpt(QStringLiteral("experimental-thread-balancer"),
                                         QCoreApplication::translate("main", "balances new connections to threads using round-robin"));
    parser.addOption(threadBalancerOpt);

    QCommandLineOption frontendProxy(QStringLiteral("using-frontend-proxy"),
                                     QCoreApplication::translate("main", "Enable frontend (reverse-)proxy support"));
    parser.addOption(frontendProxy);

    // Process the actual command line arguments given by the user
    parser.process(arguments);

    setIni(parser.values(iniOpt));

    setJson(parser.values(jsonOpt));

    if (parser.isSet(chdir)) {
        setChdir(parser.value(chdir));
    }

    if (parser.isSet(chdir2)) {
        setChdir2(parser.value(chdir2));
    }

    if (parser.isSet(threads)) {
        setThreads(parser.value(threads));
    }

    if (parser.isSet(socketAccess)) {
        setSocketAccess(parser.value(socketAccess));
    }

    if (parser.isSet(socketTimeout)) {
        bool ok;
        auto size = parser.value(socketTimeout).toInt(&ok);
        setSocketTimeout(size);
        if (!ok || size < 0) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(pidfileOpt)) {
        setPidfile(parser.value(pidfileOpt));
    }

    if (parser.isSet(pidfile2Opt)) {
        setPidfile2(parser.value(pidfile2Opt));
    }

#ifdef Q_OS_UNIX
    if (parser.isSet(stopOption)) {
        UnixFork::stopWSGI(parser.value(stopOption));
    }

    if (parser.isSet(processes)) {
        setProcesses(parser.value(processes));
    }

    if (parser.isSet(uidOption)) {
        setUid(parser.value(uidOption));
    }

    if (parser.isSet(gidOption)) {
        setGid(parser.value(gidOption));
    }

    if (parser.isSet(noInitgroupsOption)) {
        setNoInitgroups(true);
    }

    if (parser.isSet(chownSocketOption)) {
        setChownSocket(parser.value(chownSocketOption));
    }

    if (parser.isSet(umaskOption)) {
        setUmask(parser.value(umaskOption));
    }

    if (parser.isSet(cpuAffinityOption)) {
        bool ok;
        auto value = parser.value(cpuAffinityOption).toInt(&ok);
        setCpuAffinity(value);
        if (!ok || value < 0) {
            parser.showHelp(1);
        }
    }
#endif // Q_OS_UNIX

#ifdef Q_OS_LINUX
    if (parser.isSet(reusePortOption)) {
        setReusePort(true);
    }
#endif

    if (parser.isSet(lazyOption)) {
        setLazy(true);
    }

    if (parser.isSet(listenQueue)) {
        bool ok;
        auto size = parser.value(listenQueue).toInt(&ok);
        setListenQueue(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(bufferSize)) {
        bool ok;
        auto size = parser.value(bufferSize).toInt(&ok);
        setBufferSize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBuffering)) {
        bool ok;
        auto size = parser.value(postBuffering).toLongLong(&ok);
        setPostBuffering(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBufferingBufsize)) {
        bool ok;
        auto size = parser.value(postBufferingBufsize).toLongLong(&ok);
        setPostBufferingBufsize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(application)) {
        setApplication(parser.value(application));
    }

    if (parser.isSet(master)) {
        setMaster(true);
    }

    if (parser.isSet(autoReload)) {
        setAutoReload(true);
    }

    if (parser.isSet(tcpNoDelay)) {
        setTcpNodelay(true);
    }

    if (parser.isSet(soKeepAlive)) {
        setSoKeepalive(true);
    }

    if (parser.isSet(upgradeH2cOpt)) {
        setUpgradeH2c(true);
    }

    if (parser.isSet(httpsH2Opt)) {
        setHttpsH2(true);
    }

    if (parser.isSet(socketSndbuf)) {
        bool ok;
        auto size = parser.value(socketSndbuf).toInt(&ok);
        setSocketSndbuf(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(socketRcvbuf)) {
        bool ok;
        auto size = parser.value(socketRcvbuf).toInt(&ok);
        setSocketRcvbuf(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(wsMaxSize)) {
        bool ok;
        auto size = parser.value(wsMaxSize).toInt(&ok);
        setWebsocketMaxSize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(http2HeaderTableSizeOpt)) {
        bool ok;
        auto size = parser.value(http2HeaderTableSizeOpt).toUInt(&ok);
        setHttp2HeaderTableSize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(frontendProxy)) {
        setUsingFrontendProxy(true);
    }

    setHttpSocket(httpSocket() + parser.values(httpSocketOpt));

    setHttp2Socket(http2Socket() + parser.values(http2SocketOpt));

    setHttpsSocket(httpsSocket() + parser.values(httpsSocketOpt));

    setFastcgiSocket(fastcgiSocket() + parser.values(fastcgiSocketOpt));

    setStaticMap(staticMap() + parser.values(staticMapOpt));

    setStaticMap2(staticMap2() + parser.values(staticMap2Opt));

    setTouchReload(touchReload() + parser.values(touchReloadOpt));

    d->threadBalancer = parser.isSet(threadBalancerOpt);
}

int Server::exec(Cutelyst::Application *app)
{
    Q_D(Server);
    std::cout << "Cutelyst-Server starting" << std::endl;

    if (!qEnvironmentVariableIsSet("CUTELYST_SERVER_IGNORE_MASTER") && !d->master) {
        std::cout << "*** WARNING: you are running Cutelyst-Server without its master process manager ***" << std::endl;
    }

#ifdef Q_OS_UNIX
    if (d->processes == -1 && d->threads == -1) {
        d->processes = UnixFork::idealProcessCount();
        d->threads   = UnixFork::idealThreadCount() / d->processes;
    } else if (d->processes == -1) {
        d->processes = UnixFork::idealThreadCount();
    } else if (d->threads == -1) {
        d->threads = UnixFork::idealThreadCount();
    }

    if (d->processes == 0 && d->master) {
        d->processes = 1;
    }
    d->genericFork = new UnixFork(d->processes, qMax(d->threads, 1), !d->userEventLoop, this);
#else
    if (d->processes == -1) {
        d->processes = 1;
    }
    if (d->threads == -1) {
        d->threads = QThread::idealThreadCount();
    }
    d->genericFork = new WindowsFork(this);
#endif

    connect(d->genericFork, &AbstractFork::forked, d, &ServerPrivate::postFork, Qt::DirectConnection);
    connect(d->genericFork, &AbstractFork::shutdown, d, &ServerPrivate::shutdown, Qt::DirectConnection);

    if (d->master && d->lazy) {
        if (d->autoReload && !d->application.isEmpty()) {
            d->touchReload.append(d->application);
        }
        d->genericFork->setTouchReload(d->touchReload);
    }

    int ret;
    if (d->master && !d->genericFork->continueMaster(&ret)) {
        return ret;
    }

#ifdef Q_OS_LINUX
    if (systemdNotify::is_systemd_notify_available()) {
        auto sd = new systemdNotify(this);
        sd->setWatchdog(true, systemdNotify::sd_watchdog_enabled(true));
        connect(this, &Server::ready, sd, [sd] {
            sd->sendStatus(qApp->applicationName().toLatin1() + " is ready");
            sd->sendReady("1");
        });
        connect(d, &ServerPrivate::postForked, sd, [sd] {
            sd->setWatchdog(false);
        });
        qInfo(CUTELYST_SERVER) << "systemd notify detected";
    }
#endif

    // TCP needs root privileges, but SO_REUSEPORT must have an effective user ID that
    // matches the effective user ID used to perform the first bind on the socket.

    if (!d->reusePort) {
        if (!d->listenTcpSockets()) {
            Q_EMIT errorOccured(tr("No specified sockets were able to be opened"));
            return 1; // No sockets has been opened
        }
    }

    if (!d->writePidFile(d->pidfile)) {
        Q_EMIT errorOccured(QString::fromLatin1("Failed write pid file %1").arg(d->pidfile));
    }

#ifdef Q_OS_UNIX
    bool isListeningLocalSockets = false;
    if (!d->chownSocket.isEmpty()) {
        if (!d->listenLocalSockets()) {
            Q_EMIT errorOccured(QStringLiteral("Error on opening local sockets"));
            return 1;
        }
        isListeningLocalSockets = true;
    }

    if (!d->umask.isEmpty() &&
        !UnixFork::setUmask(d->umask.toLatin1())) {
        return 1;
    }

    if (!UnixFork::setGidUid(d->gid, d->uid, d->noInitgroups)) {
        Q_EMIT errorOccured(QStringLiteral("Error on setting GID or UID"));
        return 1;
    }

    if (!isListeningLocalSockets) {
#endif
        d->listenLocalSockets();
#ifdef Q_OS_UNIX
    }
#endif

    if (d->reusePort) {
        if (!d->listenTcpSockets()) {
            Q_EMIT errorOccured(tr("No specified sockets were able to be opened"));
            return 1; // No sockets has been opened
        }
    }

    if (d->servers.empty()) {
        std::cout << "Please specify a socket to listen to" << std::endl;
        Q_EMIT errorOccured(QStringLiteral("No socket specified"));
        return 1;
    }

    d->writePidFile(d->pidfile2);

    if (!d->chdir.isEmpty()) {
        std::cout << "Changing directory to: " << d->chdir.toLatin1().constData() << std::endl;
        if (!QDir::setCurrent(d->chdir)) {
            Q_EMIT errorOccured(QString::fromLatin1("Failed to chdir to: '%s'").arg(QString::fromLatin1(d->chdir.toLatin1().constData())));
            return 1;
        }
    }

    d->app = app;

    if (!d->lazy) {
        if (!d->setupApplication()) {
            Q_EMIT errorOccured(QStringLiteral("Failed to setup Application"));
            return 1;
        }
    }

    if (d->userEventLoop) {
        d->postFork(0);
        return 0;
    }

    ret = d->genericFork->exec(d->lazy, d->master);

    return ret;
}

bool Server::start(Application *app)
{
    Q_D(Server);

    if (d->engine) {
        Q_EMIT errorOccured(tr("Server not fully stopped"));
        return false;
    }

    d->processes     = 0;
    d->master        = false;
    d->lazy          = false;
    d->userEventLoop = true;
#ifdef Q_OS_UNIX
    d->uid = QString();
    d->gid = QString();
#endif
    qputenv("CUTELYST_SERVER_IGNORE_MASTER", QByteArrayLiteral("1"));

    if (exec(app) == 0) {
        return true;
    }

    return false;
}

void Server::stop()
{
    Q_D(Server);
    if (d->userEventLoop) {
        Q_EMIT d->shutdown();
    }
}

ServerPrivate::~ServerPrivate()
{
    delete protoHTTP;
    delete protoHTTP2;
    delete protoFCGI;
}

bool ServerPrivate::listenTcpSockets()
{
    if (httpSockets.isEmpty() && httpsSockets.isEmpty() && http2Sockets.isEmpty() && fastcgiSockets.isEmpty()) {
        // no sockets to listen to
        return false;
    }

    // HTTP
    for (const auto &socket : qAsConst(httpSockets)) {
        if (!listenTcp(socket, getHttpProto(), false)) {
            return false;
        }
    }

    // HTTPS
    for (const auto &socket : qAsConst(httpsSockets)) {
        if (!listenTcp(socket, getHttpProto(), true)) {
            return false;
        }
    }

    // HTTP/2
    for (const auto &socket : qAsConst(http2Sockets)) {
        if (!listenTcp(socket, getHttp2Proto(), false)) {
            return false;
        }
    }

    // FastCGI
    for (const auto &socket : qAsConst(fastcgiSockets)) {
        if (!listenTcp(socket, getFastCgiProto(), false)) {
            return false;
        }
    }

    return true;
}

bool ServerPrivate::listenTcp(const QString &line, Protocol *protocol, bool secure)
{
    Q_Q(Server);

    bool ret = true;
    if (!line.startsWith(u'/')) {
        auto server = new TcpServerBalancer(q);
        server->setBalancer(threadBalancer);
        ret = server->listen(line, protocol, secure);

        if (ret && server->socketDescriptor()) {
            auto qEnum = protocol->staticMetaObject.enumerator(0);
            std::cout << qEnum.valueToKey(static_cast<int>(protocol->type()))
                      << " socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to TCP address " << qPrintable(server->serverName())
                      << " fd " << QByteArray::number(server->socketDescriptor()).constData()
                      << std::endl;
            servers.push_back(server);
        }
    }

    return ret;
}

bool ServerPrivate::listenLocalSockets()
{
    QStringList http    = httpSockets;
    QStringList http2   = http2Sockets;
    QStringList fastcgi = fastcgiSockets;

#ifdef Q_OS_LINUX
    Q_Q(Server);

    std::vector<int> fds = systemdNotify::listenFds();
    for (int fd : fds) {
        auto server = new LocalServer(q, this);
        if (server->listen(fd)) {
            const QString name     = server->serverName();
            const QString fullName = server->fullServerName();

            Protocol *protocol;
            if (http.removeOne(fullName) || http.removeOne(name)) {
                protocol = getHttpProto();
            } else if (http2.removeOne(fullName) || http2.removeOne(name)) {
                protocol = getHttp2Proto();
            } else if (fastcgi.removeOne(fullName) || fastcgi.removeOne(name)) {
                protocol = getFastCgiProto();
            } else {
                std::cerr << "systemd activated socket does not match any configured socket" << std::endl;
                return false;
            }
            server->setProtocol(protocol);
            server->pauseAccepting();

            auto qEnum = protocol->staticMetaObject.enumerator(0);
            std::cout << qEnum.valueToKey(static_cast<int>(protocol->type()))
                      << " socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to LOCAL address " << qPrintable(fullName)
                      << " fd " << QByteArray::number(server->socket()).constData()
                      << std::endl;
            servers.push_back(server);
        } else {
            std::cerr << "Failed to listen on activated LOCAL FD: " << QByteArray::number(fd).constData()
                      << " : " << qPrintable(server->errorString()) << std::endl;
            return false;
        }
    }
#endif

    bool ret             = false;
    const auto httpConst = http;
    for (const auto &socket : httpConst) {
        ret |= listenLocal(socket, getHttpProto());
    }

    const auto http2Const = http2;
    for (const auto &socket : http2Const) {
        ret |= listenLocal(socket, getHttp2Proto());
    }

    const auto fastcgiConst = fastcgi;
    for (const auto &socket : fastcgiConst) {
        ret |= listenLocal(socket, getFastCgiProto());
    }

    return ret;
}

bool ServerPrivate::listenLocal(const QString &line, Protocol *protocol)
{
    Q_Q(Server);

    bool ret = true;
    if (line.startsWith(QLatin1Char('/'))) {
        auto server = new LocalServer(q, this);
        server->setProtocol(protocol);
        if (!socketAccess.isEmpty()) {
            QLocalServer::SocketOptions options;
            if (socketAccess.contains(u'u')) {
                options |= QLocalServer::UserAccessOption;
            }

            if (socketAccess.contains(u'g')) {
                options |= QLocalServer::GroupAccessOption;
            }

            if (socketAccess.contains(u'o')) {
                options |= QLocalServer::OtherAccessOption;
            }
            server->setSocketOptions(options);
        }
        server->removeServer(line);
        ret = server->listen(line);
        server->pauseAccepting();

        if (!ret || !server->socket()) {
            std::cerr << "Failed to listen on LOCAL: " << qPrintable(line)
                      << " : " << qPrintable(server->errorString()) << std::endl;
            return false;
        }

#ifdef Q_OS_UNIX
        if (!chownSocket.isEmpty()) {
            UnixFork::chownSocket(line, chownSocket);
        }
#endif
        auto qEnum = protocol->staticMetaObject.enumerator(0);
        std::cout << qEnum.valueToKey(static_cast<int>(protocol->type()))
                  << " socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                  << " bound to LOCAL address " << qPrintable(line)
                  << " fd " << QByteArray::number(server->socket()).constData()
                  << std::endl;
        servers.push_back(server);
    }

    return ret;
}

void Server::setApplication(const QString &application)
{
    Q_D(Server);

    QPluginLoader loader(application);
    if (loader.fileName().isEmpty()) {
        d->application = application;
    } else {
        // We use the loader filename since it can provide
        // the suffix for the file watcher
        d->application = loader.fileName();
    }
    Q_EMIT changed();
}

QString Server::application() const
{
    Q_D(const Server);
    return d->application;
}

void Server::setThreads(const QString &threads)
{
    Q_D(Server);
    if (threads.compare(u"auto", Qt::CaseInsensitive) == 0) {
        d->threads = -1;
    } else {
        d->threads = qMax(1, threads.toInt());
    }
    Q_EMIT changed();
}

QString Server::threads() const
{
    Q_D(const Server);
    if (d->threads == -1) {
        return QStringLiteral("auto");
    }
    return QString::number(d->threads);
}

void Server::setProcesses(const QString &process)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    if (process.compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0) {
        d->processes = -1;
    } else {
        d->processes = process.toInt();
    }
    Q_EMIT changed();
#endif
}

QString Server::processes() const
{
    Q_D(const Server);
    if (d->processes == -1) {
        return QStringLiteral("auto");
    }
    return QString::number(d->processes);
}

void Server::setChdir(const QString &chdir)
{
    Q_D(Server);
    d->chdir = chdir;
    Q_EMIT changed();
}

QString Server::chdir() const
{
    Q_D(const Server);
    return d->chdir;
}

void Server::setHttpSocket(const QStringList &httpSocket)
{
    Q_D(Server);
    d->httpSockets = httpSocket;
    Q_EMIT changed();
}

QStringList Server::httpSocket() const
{
    Q_D(const Server);
    return d->httpSockets;
}

void Server::setHttp2Socket(const QStringList &http2Socket)
{
    Q_D(Server);
    d->http2Sockets = http2Socket;
    Q_EMIT changed();
}

QStringList Server::http2Socket() const
{
    Q_D(const Server);
    return d->http2Sockets;
}

void Server::setHttp2HeaderTableSize(quint32 headerTableSize)
{
    Q_D(Server);
    d->http2HeaderTableSize = headerTableSize;
    Q_EMIT changed();
}

quint32 Server::http2HeaderTableSize() const
{
    Q_D(const Server);
    return d->http2HeaderTableSize;
}

void Server::setUpgradeH2c(bool enable)
{
    Q_D(Server);
    d->upgradeH2c = enable;
    Q_EMIT changed();
}

bool Server::upgradeH2c() const
{
    Q_D(const Server);
    return d->upgradeH2c;
}

void Server::setHttpsH2(bool enable)
{
    Q_D(Server);
    d->httpsH2 = enable;
    Q_EMIT changed();
}

bool Server::httpsH2() const
{
    Q_D(const Server);
    return d->httpsH2;
}

void Server::setHttpsSocket(const QStringList &httpsSocket)
{
    Q_D(Server);
    d->httpsSockets = httpsSocket;
    Q_EMIT changed();
}

QStringList Server::httpsSocket() const
{
    Q_D(const Server);
    return d->httpsSockets;
}

void Server::setFastcgiSocket(const QStringList &fastcgiSocket)
{
    Q_D(Server);
    d->fastcgiSockets = fastcgiSocket;
    Q_EMIT changed();
}

QStringList Server::fastcgiSocket() const
{
    Q_D(const Server);
    return d->fastcgiSockets;
}

void Server::setSocketAccess(const QString &socketAccess)
{
    Q_D(Server);
    d->socketAccess = socketAccess;
    Q_EMIT changed();
}

QString Server::socketAccess() const
{
    Q_D(const Server);
    return d->socketAccess;
}

void Server::setSocketTimeout(int timeout)
{
    Q_D(Server);
    d->socketTimeout = timeout;
    Q_EMIT changed();
}

int Server::socketTimeout() const
{
    Q_D(const Server);
    return d->socketTimeout;
}

void Server::setChdir2(const QString &chdir2)
{
    Q_D(Server);
    d->chdir2 = chdir2;
    Q_EMIT changed();
}

QString Server::chdir2() const
{
    Q_D(const Server);
    return d->chdir2;
}

void Server::setIni(const QStringList &files)
{
    Q_D(Server);
    d->ini.append(files);
    d->ini.removeDuplicates();
    Q_EMIT changed();

    for (const QString &ini : d->ini) {
        d->loadConfig(ini, false);
    }
}

QStringList Server::ini() const
{
    Q_D(const Server);
    return d->ini;
}

void Server::setJson(const QStringList &files)
{
    Q_D(Server);
    d->json.append(files);
    d->json.removeDuplicates();
    Q_EMIT changed();

    for (const QString &json : d->json) {
        d->loadConfig(json, true);
    }
}

QStringList Server::json() const
{
    Q_D(const Server);
    return d->json;
}

void Server::setStaticMap(const QStringList &staticMap)
{
    Q_D(Server);
    d->staticMaps = staticMap;
    Q_EMIT changed();
}

QStringList Server::staticMap() const
{
    Q_D(const Server);
    return d->staticMaps;
}

void Server::setStaticMap2(const QStringList &staticMap)
{
    Q_D(Server);
    d->staticMaps2 = staticMap;
    Q_EMIT changed();
}

QStringList Server::staticMap2() const
{
    Q_D(const Server);
    return d->staticMaps2;
}

void Server::setMaster(bool enable)
{
    Q_D(Server);
    if (!qEnvironmentVariableIsSet("CUTELYST_SERVER_IGNORE_MASTER")) {
        d->master = enable;
    }
    Q_EMIT changed();
}

bool Server::master() const
{
    Q_D(const Server);
    return d->master;
}

void Server::setAutoReload(bool enable)
{
    Q_D(Server);
    if (enable) {
        d->autoReload = true;
    }
    Q_EMIT changed();
}

bool Server::autoReload() const
{
    Q_D(const Server);
    return d->autoReload;
}

void Server::setTouchReload(const QStringList &files)
{
    Q_D(Server);
    d->touchReload = files;
    Q_EMIT changed();
}

QStringList Server::touchReload() const
{
    Q_D(const Server);
    return d->touchReload;
}

void Server::setListenQueue(int size)
{
    Q_D(Server);
    d->listenQueue = size;
    Q_EMIT changed();
}

int Server::listenQueue() const
{
    Q_D(const Server);
    return d->listenQueue;
}

void Server::setBufferSize(int size)
{
    Q_D(Server);
    if (size < 4096) {
        qCWarning(CUTELYST_SERVER) << "Buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    d->bufferSize = size;
    Q_EMIT changed();
}

int Server::bufferSize() const
{
    Q_D(const Server);
    return d->bufferSize;
}

void Server::setPostBuffering(qint64 size)
{
    Q_D(Server);
    d->postBuffering = size;
    Q_EMIT changed();
}

qint64 Server::postBuffering() const
{
    Q_D(const Server);
    return d->postBuffering;
}

void Server::setPostBufferingBufsize(qint64 size)
{
    Q_D(Server);
    if (size < 4096) {
        qCWarning(CUTELYST_SERVER) << "Post buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    d->postBufferingBufsize = size;
    Q_EMIT changed();
}

qint64 Server::postBufferingBufsize() const
{
    Q_D(const Server);
    return d->postBufferingBufsize;
}

void Server::setTcpNodelay(bool enable)
{
    Q_D(Server);
    d->tcpNodelay = enable;
    Q_EMIT changed();
}

bool Server::tcpNodelay() const
{
    Q_D(const Server);
    return d->tcpNodelay;
}

void Server::setSoKeepalive(bool enable)
{
    Q_D(Server);
    d->soKeepalive = enable;
    Q_EMIT changed();
}

bool Server::soKeepalive() const
{
    Q_D(const Server);
    return d->soKeepalive;
}

void Server::setSocketSndbuf(int value)
{
    Q_D(Server);
    d->socketSendBuf = value;
    Q_EMIT changed();
}

int Server::socketSndbuf() const
{
    Q_D(const Server);
    return d->socketSendBuf;
}

void Server::setSocketRcvbuf(int value)
{
    Q_D(Server);
    d->socketReceiveBuf = value;
    Q_EMIT changed();
}

int Server::socketRcvbuf() const
{
    Q_D(const Server);
    return d->socketReceiveBuf;
}

void Server::setWebsocketMaxSize(int value)
{
    Q_D(Server);
    d->websocketMaxSize = value * 1024;
    Q_EMIT changed();
}

int Server::websocketMaxSize() const
{
    Q_D(const Server);
    return d->websocketMaxSize / 1024;
}

void Server::setPidfile(const QString &file)
{
    Q_D(Server);
    d->pidfile = file;
    Q_EMIT changed();
}

QString Server::pidfile() const
{
    Q_D(const Server);
    return d->pidfile;
}

void Server::setPidfile2(const QString &file)
{
    Q_D(Server);
    d->pidfile2 = file;
    Q_EMIT changed();
}

QString Server::pidfile2() const
{
    Q_D(const Server);
    return d->pidfile2;
}

void Server::setUid(const QString &uid)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->uid = uid;
    Q_EMIT changed();
#endif
}

QString Server::uid() const
{
    Q_D(const Server);
    return d->uid;
}

void Server::setGid(const QString &gid)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->gid = gid;
    Q_EMIT changed();
#endif
}

QString Server::gid() const
{
    Q_D(const Server);
    return d->gid;
}

void Server::setNoInitgroups(bool enable)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->noInitgroups = enable;
    Q_EMIT changed();
#endif
}

bool Server::noInitgroups() const
{
    Q_D(const Server);
    return d->noInitgroups;
}

void Server::setChownSocket(const QString &chownSocket)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->chownSocket = chownSocket;
    Q_EMIT changed();
#endif
}

QString Server::chownSocket() const
{
    Q_D(const Server);
    return d->chownSocket;
}

void Server::setUmask(const QString &value)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->umask = value;
    Q_EMIT changed();
#endif
}

QString Server::umask() const
{
    Q_D(const Server);
    return d->umask;
}

void Server::setCpuAffinity(int value)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    d->cpuAffinity = value;
    Q_EMIT changed();
#endif
}

int Server::cpuAffinity() const
{
    Q_D(const Server);
    return d->cpuAffinity;
}

void Server::setReusePort(bool enable)
{
#ifdef Q_OS_LINUX
    Q_D(Server);
    d->reusePort = enable;
    Q_EMIT changed();
#endif
}

bool Server::reusePort() const
{
    Q_D(const Server);
    return d->reusePort;
}

void Server::setLazy(bool enable)
{
    Q_D(Server);
    d->lazy = enable;
    Q_EMIT changed();
}

bool Server::lazy() const
{
    Q_D(const Server);
    return d->lazy;
}

void Server::setUsingFrontendProxy(bool enable)
{
    Q_D(Server);
    d->usingFrontendProxy = enable;
    Q_EMIT changed();
}

bool Server::usingFrontendProxy() const
{
    Q_D(const Server);
    return d->usingFrontendProxy;
}

bool ServerPrivate::setupApplication()
{
    Cutelyst::Application *localApp = app;

    Q_Q(Server);

    if (!localApp) {
        std::cout << "Loading application: " << application.toLatin1().constData() << std::endl;
        QPluginLoader loader(application);
        if (!loader.load()) {
            qCCritical(CUTELYST_SERVER) << "Could not load application:" << loader.errorString();
            return false;
        }

        QObject *instance = loader.instance();
        if (!instance) {
            qCCritical(CUTELYST_SERVER) << "Could not get a QObject instance: %s\n"
                                        << loader.errorString();
            return false;
        }

        localApp = qobject_cast<Cutelyst::Application *>(instance);
        if (!localApp) {
            qCCritical(CUTELYST_SERVER) << "Could not cast Cutelyst::Application from instance: %s\n"
                                        << loader.errorString();
            return false;
        }

        // Sets the application name with the name from our library
        //    if (QCoreApplication::applicationName() == applicationName) {
        //        QCoreApplication::setApplicationName(QString::fromLatin1(app->metaObject()->className()));
        //    }
        qCDebug(CUTELYST_SERVER) << "Loaded application: " << QCoreApplication::applicationName();
    }

    if (!chdir2.isEmpty()) {
        std::cout << "Changing directory2 to: " << chdir2.toLatin1().constData() << std::endl;
        if (!QDir::setCurrent(chdir2)) {
            Q_EMIT q->errorOccured(QString::fromLatin1("Failed to chdir2 to: '%s'").arg(QString::fromLatin1(chdir2.toLatin1().constData())));
            return false;
        }
    }

    if (threads > 1) {
        engine = createEngine(localApp, 0);
        for (int i = 1; i < threads; ++i) {
            if (createEngine(localApp, i)) {
                ++workersNotRunning;
            }
        }
    } else {
        engine            = createEngine(localApp, 0);
        workersNotRunning = 1;
    }

    if (!engine) {
        std::cerr << "Application failed to init, cheaping..." << std::endl;
        return false;
    }

    return true;
}

void ServerPrivate::engineShutdown(CWsgiEngine *engine)
{
    const auto engineThread = engine->thread();
    if (QThread::currentThread() != engineThread) {
        connect(engineThread, &QThread::finished, this, [this, engine] {
            engines.erase(std::remove(engines.begin(), engines.end(), engine), engines.end());
            checkEngineShutdown();
        });
        engineThread->quit();
    } else {
        engines.erase(std::remove(engines.begin(), engines.end(), engine), engines.end());
    }

    checkEngineShutdown();
}

void ServerPrivate::checkEngineShutdown()
{
    if (engines.empty()) {
        if (userEventLoop) {
            Q_Q(Server);
            Q_EMIT q->stopped();
        } else {
            QTimer::singleShot(0, this, [] {
                qApp->exit(15);
            });
        }
    }
}

void ServerPrivate::workerStarted()
{
    Q_Q(Server);

    // All workers have started
    if (--workersNotRunning == 0) {
        Q_EMIT q->ready();
    }
}

bool ServerPrivate::postFork(int workerId)
{
    Q_Q(Server);

    if (lazy) {
        if (!setupApplication()) {
            Q_EMIT q->errorOccured(QStringLiteral("Failed to setup Application"));
            return false;
        }
    }

    if (engines.size() > 1) {
        qCDebug(CUTELYST_SERVER) << "Starting threads";
    }

    for (CWsgiEngine *engine : engines) {
        QThread *thread = engine->thread();
        if (thread != qApp->thread()) {
#ifdef Q_OS_LINUX
            if (!qEnvironmentVariableIsSet("CUTELYST_QT_EVENT_LOOP")) {
                thread->setEventDispatcher(new EventDispatcherEPoll);
            }
#endif

            thread->start();
        }
    }

    Q_EMIT postForked(workerId);

    QTimer::singleShot(1000, this, [=]() {
        // THIS IS NEEDED when
        // --master --threads N --experimental-thread-balancer
        // for some reason sometimes the balancer doesn't get
        // the ready signal (which stays on event loop queue)
        // from TcpServer and doesn't starts listening.
        qApp->processEvents();
    });

    return true;
}

bool ServerPrivate::writePidFile(const QString &filename)
{
    if (filename.isEmpty()) {
        return true;
    }

    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Failed write pid file " << qPrintable(filename) << std::endl;
        return false;
    }

    std::cout << "Writing pidfile to " << qPrintable(filename) << std::endl;
    file.write(QByteArray::number(QCoreApplication::applicationPid()) + '\n');

    return true;
}

CWsgiEngine *ServerPrivate::createEngine(Application *app, int workerCore)
{
    Q_Q(Server);

    // If threads is greater than 1 we need a new application instance
    if (workerCore > 0) {
        app = qobject_cast<Application *>(app->metaObject()->newInstance());
        if (!app) {
            qFatal("*** FATAL *** Could not create a NEW instance of your Cutelyst::Application, "
                   "make sure your constructor has Q_INVOKABLE macro or disable threaded mode.");
        }
    }

    auto engine = new CWsgiEngine(app, workerCore, opt, q);
    connect(this, &ServerPrivate::shutdown, engine, &CWsgiEngine::shutdown, Qt::QueuedConnection);
    connect(this, &ServerPrivate::postForked, engine, &CWsgiEngine::postFork, Qt::QueuedConnection);
    connect(engine, &CWsgiEngine::shutdownCompleted, this, &ServerPrivate::engineShutdown, Qt::QueuedConnection);
    connect(engine, &CWsgiEngine::started, this, &ServerPrivate::workerStarted, Qt::QueuedConnection);

    engine->setConfig(config);
    engine->setServers(servers);
    if (!engine->init()) {
        std::cerr << "Application failed to init(), cheaping core: " << workerCore << std::endl;
        delete engine;
        return nullptr;
    }

    engines.push_back(engine);

    // If threads is greater than 1 we need a new thread
    if (workerCore > 0) {
        // To make easier for engines to clean up
        // the NEW app must be a child of it
        app->setParent(engine);

        auto thread = new QThread(this);
        engine->moveToThread(thread);
    } else {
        engine->setParent(this);
    }

    return engine;
}

void ServerPrivate::loadConfig(const QString &file, bool json)
{
    if (configLoaded.contains(file)) {
        return;
    }

    QString filename = file;
    configLoaded.append(file);

    QString section = QStringLiteral("server");
    if (filename.contains(QLatin1Char(':'))) {
        section  = filename.section(QLatin1Char(':'), -1, 1);
        filename = filename.section(QLatin1Char(':'), 0, -2);
    }

    QVariantMap loadedConfig;
    if (json) {
        std::cout << "Loading JSON configuration: " << qPrintable(filename)
                  << " section: " << qPrintable(section) << std::endl;
        loadedConfig = Engine::loadJsonConfig(filename);
    } else {
        std::cout << "Loading INI configuration: " << qPrintable(filename)
                  << " section: " << qPrintable(section) << std::endl;
        loadedConfig = Engine::loadIniConfig(filename);
    }

    QVariantMap sessionConfig = loadedConfig.value(section).toMap();

    applyConfig(sessionConfig);

    sessionConfig.insert(opt);
    opt = sessionConfig;

    auto it = config.begin();
    while (it != config.end()) {
        auto itLoaded = loadedConfig.find(it.key());
        while (itLoaded != loadedConfig.end()) {
            QVariantMap loadedMap = itLoaded.value().toMap();
            loadedMap.insert(it.value().toMap());

            it.value() = loadedMap;
            itLoaded   = loadedConfig.erase(itLoaded);
        }
        ++it;
    }

    loadedConfig.insert(config);

    config = loadedConfig;
}

void ServerPrivate::applyConfig(const QVariantMap &config)
{
    Q_Q(Server);

    auto it = config.constBegin();
    while (it != config.constEnd()) {
        QString normKey = it.key();
        normKey.replace(u'-', u'_');

        int ix = q->metaObject()->indexOfProperty(normKey.toLatin1().constData());
        if (ix == -1) {
            ++it;
            continue;
        }

        const QVariant value     = it.value();
        const QMetaProperty prop = q->metaObject()->property(ix);
        if (prop.userType() == value.userType()) {
            if (prop.userType() == QMetaType::QStringList) {
                const QStringList currentValues = prop.read(q).toStringList();
                prop.write(q, currentValues + value.toStringList());
            } else {
                prop.write(q, value);
            }
        } else if (prop.userType() == QMetaType::QStringList) {
            const QStringList currentValues = prop.read(q).toStringList();
            prop.write(q, currentValues + QStringList{value.toString()});
        } else {
            prop.write(q, value);
        }

        ++it;
    }
}

Protocol *ServerPrivate::getHttpProto()
{
    Q_Q(Server);
    if (!protoHTTP) {
        if (upgradeH2c) {
            protoHTTP = new ProtocolHttp(q, getHttp2Proto());
        } else {
            protoHTTP = new ProtocolHttp(q);
        }
    }
    return protoHTTP;
}

ProtocolHttp2 *ServerPrivate::getHttp2Proto()
{
    Q_Q(Server);
    if (!protoHTTP2) {
        protoHTTP2 = new ProtocolHttp2(q);
    }
    return protoHTTP2;
}

Protocol *ServerPrivate::getFastCgiProto()
{
    Q_Q(Server);
    if (!protoFCGI) {
        protoFCGI = new ProtocolFastCGI(q);
    }
    return protoFCGI;
}

#include "moc_server.cpp"
#include "moc_server_p.cpp"
