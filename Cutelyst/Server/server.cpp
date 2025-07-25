/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "localserver.h"
#include "protocol.h"
#include "protocolfastcgi.h"
#include "protocolhttp.h"
#include "protocolhttp2.h"
#include "server_p.h"
#include "serverengine.h"
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
using namespace Qt::Literals::StringLiterals;

Server::Server(QObject *parent)
    : QObject(parent)
    , d_ptr(new ServerPrivate(this))
{
    QCoreApplication::addLibraryPath(QDir().absolutePath());

    if (!qEnvironmentVariableIsSet("QT_MESSAGE_PATTERN")) {
        if (qEnvironmentVariableIsSet("JOURNAL_STREAM")) {
            // systemd journal already logs PID, check if it logs threadid as well
            qSetMessagePattern(u"%{category}[%{type}] %{message}"_s);
        } else {
            qSetMessagePattern(u"%{pid}:%{threadid} %{category}[%{type}] %{message}"_s);
        }
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

        delete d->mainEngine;
        d->mainEngine = nullptr;

        qDeleteAll(d->servers);
        d->servers.clear();
    };

    connect(this, &Server::errorOccured, this, cleanUp);
    connect(this, &Server::stopped, this, cleanUp);
}

Server::~Server()
{
    delete d_ptr;
    std::cout << "Cutelyst-Server terminated" << '\n';
}

void Server::parseCommandLine(const QStringList &arguments)
{
    Q_D(Server);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        //: CLI app description
        //% "Fast, developer-friendly server."
        qtTrId("cutelystd-cli-desc"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniOpt(u"ini"_s,
                              //: CLI option description
                              //% "Load config from INI file. When used multiple times, content "
                              //% "will be merged and same keys in the sections will be "
                              //% "overwritten by content from later files."
                              qtTrId("cutelystd-opt-ini-desc"),
                              //: CLI option value name
                              //% "file"
                              qtTrId("cutelystd-opt-value-file"));
    parser.addOption(iniOpt);

    QCommandLineOption jsonOpt({u"j"_s, u"json"_s},
                               //: CLI option description
                               //% "Load config from JSON file. When used multiple times, content "
                               //% "will be merged and same keys in the sections will be "
                               //% "overwritten by content from later files."
                               qtTrId("cutelystd-opt-json-desc"),
                               qtTrId("cutelystd-opt-value-file"));
    parser.addOption(jsonOpt);

    QCommandLineOption chdirOpt(
        u"chdir"_s,
        //: CLI option description
        //% "Change to the specified directory before the application is loaded."
        qtTrId("cutelystd-opt-chdir-desc"),
        //: CLI option value name
        //% "directory"
        qtTrId("cutelystd-opt-value-directory"));
    parser.addOption(chdirOpt);

    QCommandLineOption chdir2Opt(
        u"chdir2"_s,
        //: CLI option description
        //% "Change to the specified directory after the application has been loaded."
        qtTrId("cutelystd-opt-chdir2-desc"),
        qtTrId("cutelystd-opt-value-directory"));
    parser.addOption(chdir2Opt);

    QCommandLineOption lazyOpt(
        u"lazy"_s,
        //: CLI option description
        //% "Use lazy mode (load the application in the workers instead of master)."
        qtTrId("cutelystd-opt-lazy-desc"));
    parser.addOption(lazyOpt);

    QCommandLineOption applicationOpt({u"application"_s, u"a"_s},
                                      //: CLI option description
                                      //% "Path to the application file to load."
                                      qtTrId("cutelystd-opt-application-desc"),
                                      qtTrId("cutelystd-opt-value-file"));
    parser.addOption(applicationOpt);

    QCommandLineOption threadsOpt({u"threads"_s, u"t"_s},
                                  //: CLI option description
                                  //% "The number of threads to use. If set to “auto”, the ideal "
                                  //% "thread count is used."
                                  qtTrId("cutelystd-opt-threads-desc"),
                                  //: CLI option value name
                                  //% "threads"
                                  qtTrId("cutelystd-opt-threads-value"));
    parser.addOption(threadsOpt);

#ifdef Q_OS_UNIX
    QCommandLineOption processesOpt({u"processes"_s, u"p"_s},
                                    //: CLI option description
                                    //% "Spawn the specified number of processes. If set to “auto”,
                                    //" % "the ideal process count is used."
                                    qtTrId("cutelystd-opt-processes-desc"),
                                    //: CLI option value name
                                    //% "processes"
                                    qtTrId("cutelystd-opt-processes-value"));
    parser.addOption(processesOpt);
#endif

    QCommandLineOption masterOpt({u"master"_s, u"M"_s},
                                 //: CLI option description
                                 //% "Enable master process."
                                 qtTrId("cutelystd-opt-master-desc"));
    parser.addOption(masterOpt);

    QCommandLineOption listenQueueOpt({u"listen"_s, u"l"_s},
                                      //: CLI option description
                                      //% "Set the socket listen queue size. Default value: 100."
                                      qtTrId("cutelystd-opt-listen-desc"),
                                      //: CLI option value name
                                      //% "size"
                                      qtTrId("cutelystd-opt-value-size"));
    parser.addOption(listenQueueOpt);

    QCommandLineOption bufferSizeOpt({u"buffer-size"_s, u"b"_s},
                                     //: CLI option description
                                     //% "Set the internal buffer size. Default value: 4096."
                                     qtTrId("cutelystd-opt-buffer-size-desc"),
                                     //: CLI option value name
                                     //% "bytes"
                                     qtTrId("cutelystd-opt-value-bytes"));
    parser.addOption(bufferSizeOpt);

    QCommandLineOption postBufferingOpt(
        u"post-buffering"_s,
        //: CLI option description
        //% "Sets the size after which buffering takes place on the "
        //% "hard disk instead of in the main memory. "
        //% "Default value: -1."
        qtTrId("cutelystd-opt-post-buffering-desc"),
        qtTrId("cutelystd-opt-value-bytes"));
    parser.addOption(postBufferingOpt);

    QCommandLineOption postBufferingBufsizeOpt(
        u"post-buffering-bufsize"_s,
        //: CLI option description
        //% "Set the buffer size for read() in post buffering mode. Default value: 4096."
        qtTrId("cutelystd-opt-post-buffering-bufsize-desc"),
        qtTrId("cutelystd-opt-value-bytes"));
    parser.addOption(postBufferingBufsizeOpt);

    QCommandLineOption httpSocketOpt({u"http-socket"_s, u"h1"_s},
                                     //: CLI option description
                                     //% "Bind to the specified TCP socket using the HTTP protocol."
                                     qtTrId("cutelystd-opt-http-socket-desc"),
                                     //: CLI option value name
                                     //% "[address]:port"
                                     qtTrId("cutelystd-opt-value-address"));
    parser.addOption(httpSocketOpt);

    QCommandLineOption http2SocketOpt(
        {u"http2-socket"_s, u"h2"_s},
        //: CLI option description
        //% "Bind to the specified TCP socket using the HTTP/2 Clear Text protocol."
        qtTrId("cutelystd-opt-http2-socket-desc"),
        qtTrId("cutelystd-opt-value-address"));
    parser.addOption(http2SocketOpt);

    QCommandLineOption http2HeaderTableSizeOpt(u"http2-header-table-size"_s,
                                               //: CLI option description
                                               //% "Sets the HTTP/2 header table size."
                                               qtTrId("cutelystd-opt-http2-header-table-size-desc"),
                                               qtTrId("cutelystd-opt-value-size"));
    parser.addOption(http2HeaderTableSizeOpt);

    QCommandLineOption upgradeH2cOpt(u"upgrade-h2c"_s,
                                     //: CLI option description
                                     //% "Upgrades HTTP/1 to H2c (HTTP/2 Clear Text)."
                                     qtTrId("cutelystd-opt-upgrade-h2c-desc"));
    parser.addOption(upgradeH2cOpt);

    QCommandLineOption httpsH2Opt(u"https-h2"_s,
                                  //: CLI option description
                                  //% "Negotiate HTTP/2 on HTTPS socket."
                                  qtTrId("cutelystd-opt-https-h2-desc"));
    parser.addOption(httpsH2Opt);

    QCommandLineOption httpsSocketOpt({u"https-socket"_s, u"hs1"_s},
                                      //: CLI option description
                                      //% "Bind to the specified TCP socket using HTTPS protocol."
                                      qtTrId("cutelystd-opt-https-socket-desc"),
                                      //% "[address]:port,certFile,keyFile[,algorithm]"
                                      qtTrId("cutelystd-opt-value-httpsaddress"));
    parser.addOption(httpsSocketOpt);

    QCommandLineOption fastcgiSocketOpt(
        u"fastcgi-socket"_s,
        //: CLI option description
        //% "Bind to the specified UNIX/TCP socket using FastCGI protocol."
        qtTrId("cutelystd-opt-fastcgi-socket-desc"),
        qtTrId("cutelystd-opt-value-address"));
    parser.addOption(fastcgiSocketOpt);

    QCommandLineOption socketAccessOpt(
        u"socket-access"_s,
        //: CLI option description
        //% "Set the LOCAL socket access, such as 'ugo' standing for User, Group, Other access."
        qtTrId("cutelystd-opt-socket-access-desc"),
        //: CLI option value name
        //% "options"
        qtTrId("cutelystd-opt-socket-access-value"));
    parser.addOption(socketAccessOpt);

    QCommandLineOption socketTimeoutOpt({u"socket-timeout"_s, u"z"_s},
                                        //: CLI option description
                                        //% "Set internal socket timeouts. Default value: 4."
                                        qtTrId("cutelystd-opt-socket-timeout-desc"),
                                        //: CLI option value name
                                        //% "seconds"
                                        qtTrId("cutelystd-opt-socket-timeout-value"));
    parser.addOption(socketTimeoutOpt);

    QCommandLineOption staticMapOpt(u"static-map"_s,
                                    //: CLI option description
                                    //% "Map mountpoint to local directory to serve static files. "
                                    //% "The mountpoint will be removed from the request path and "
                                    //% "the rest will be appended to the local path to find the "
                                    //% "file to serve. Can be used multiple times."
                                    qtTrId("cutelystd-opt-static-map-desc"),
                                    //: CLI option value name
                                    //% "/mountpoint=/path"
                                    qtTrId("cutelystd-opt-value-static-map"));
    parser.addOption(staticMapOpt);

    QCommandLineOption staticMap2Opt(u"static-map2"_s,
                                     //: CLI option description
                                     //% "Like static-map but completely appending the request "
                                     //% "path to the local path. Can be used multiple times."
                                     qtTrId("cutelystd-opt-static-map2-desc"),
                                     //: CLI option value name
                                     //% "/mountpoint=/path"
                                     qtTrId("cutelystd-opt-value-static-map"));
    parser.addOption(staticMap2Opt);

    QCommandLineOption autoReloadOpt({u"auto-restart"_s, u"r"_s},
                                     //: CLI option description
                                     //% "Auto restarts when the application file changes. Master "
                                     //% "process and lazy mode have to be enabled."
                                     qtTrId("cutelystd-opt-auto-restart-desc"));
    parser.addOption(autoReloadOpt);

    QCommandLineOption touchReloadOpt(
        u"touch-reload"_s,
        //: CLI option description
        //% "Reload the application if the specified file is modified/touched. Master process "
        //% "and lazy mode have to be enabled."
        qtTrId("cutelystd-opt-touch-reload-desc"),
        qtTrId("cutelystd-opt-value-file"));
    parser.addOption(touchReloadOpt);

    QCommandLineOption tcpNoDelay(u"tcp-nodelay"_s,
                                  //: CLI option description
                                  //% "Enable TCP NODELAY on each request."
                                  qtTrId("cutelystd-opt-tcp-nodelay-desc"));
    parser.addOption(tcpNoDelay);

    QCommandLineOption soKeepAlive(u"so-keepalive"_s,
                                   //: CLI option description
                                   //% "Enable TCP KEEPALIVE."
                                   qtTrId("cutelystd-opt-so-keepalive-desc"));
    parser.addOption(soKeepAlive);

    QCommandLineOption socketSndbufOpt(u"socket-sndbuf"_s,
                                       //: CLI option description
                                       //% "Sets the socket send buffer size in bytes at the OS "
                                       //% "level. This maps to the SO_SNDBUF socket option."
                                       qtTrId("cutelystd-opt-socket-sndbuf-desc"),
                                       qtTrId("cutelystd-opt-value-bytes"));
    parser.addOption(socketSndbufOpt);

    QCommandLineOption socketRcvbufOpt(u"socket-rcvbuf"_s,
                                       //: CLI option description
                                       //% "Sets the socket receive buffer size in bytes at the OS "
                                       //% "level. This maps to the SO_RCVBUF socket option."
                                       qtTrId("cutelystd-opt-socket-rcvbuf-desc"),
                                       qtTrId("cutelystd-opt-value-bytes"));
    parser.addOption(socketRcvbufOpt);

    QCommandLineOption wsMaxSize(u"websocket-max-size"_s,
                                 //: CLI option description
                                 //% "Maximum allowed payload size for websocket in kibibytes. "
                                 //% "Default value: 1024 KiB."
                                 qtTrId("cutelystd-opt-websocket-max-size-desc"),
                                 //: CLI option value name
                                 //% "kibibyte"
                                 qtTrId("cutelystd-opt-websocket-max-size-value"));
    parser.addOption(wsMaxSize);

    QCommandLineOption pidfileOpt(u"pidfile"_s,
                                  //: CLI option description
                                  //% "Create pidfile (before privilege drop)."
                                  qtTrId("cutelystd-opt-pidfile-desc"),
                                  //: CLI option value name
                                  //% "pidfile"
                                  qtTrId("cutelystd-opt-value-pidfile"));
    parser.addOption(pidfileOpt);

    QCommandLineOption pidfile2Opt(u"pidfile2"_s,
                                   //: CLI option description
                                   //% "Create pidfile (after privilege drop)."
                                   qtTrId("cutelystd-opt-pidfile2-desc"),
                                   qtTrId("cutelystd-opt-value-pidfile"));
    parser.addOption(pidfile2Opt);

#ifdef Q_OS_UNIX
    QCommandLineOption stopOpt(u"stop"_s,
                               //: CLI option description
                               //% "Stop an instance identified by the PID in the pidfile."
                               qtTrId("cutelystd-opt-stop-desc"),
                               qtTrId("cutelystd-opt-value-pidfile"));
    parser.addOption(stopOpt);

    QCommandLineOption uidOpt(u"uid"_s,
                              //: CLI option description
                              //% "Setuid to the specified user/uid."
                              qtTrId("cutelystd-opt-uid-desc"),
                              //: CLI option value name
                              //% "user/uid"
                              qtTrId("cutelystd-opt-uid-value"));
    parser.addOption(uidOpt);

    QCommandLineOption gidOpt(u"gid"_s,
                              //: CLI option description
                              //% "Setuid to the specified group/gid."
                              qtTrId("cutelystd-opt-gid-desc"),
                              //: CLI option value name
                              //% "group/gid"
                              qtTrId("cutelystd-opt-gid-value"));
    parser.addOption(gidOpt);

    QCommandLineOption noInitgroupsOpt(u"no-initgroups"_s,
                                       //: CLI option description
                                       //% "Disable additional groups set via initgroups()."
                                       qtTrId("cutelystd-opt-no-init-groups-desc"));
    parser.addOption(noInitgroupsOpt);

    QCommandLineOption chownSocketOpt(u"chown-socket"_s,
                                      //: CLI option description
                                      //% "Change the ownership of the UNIX socket."
                                      qtTrId("cutelystd-opt-chown-socket-desc"),
                                      //: CLI option value name
                                      //% "uid:gid"
                                      qtTrId("cutelystd-opt-chown-socket-value"));
    parser.addOption(chownSocketOpt);

    QCommandLineOption umaskOpt(u"umask"_s,
                                //: CLI option description
                                //% "Set file mode creation mask."
                                qtTrId("cutelystd-opt-umask-desc"),
                                //: CLI option value name
                                //% "mask"
                                qtTrId("cutelystd-opt-umask-value"));
    parser.addOption(umaskOpt);

    QCommandLineOption cpuAffinityOpt(
        u"cpu-affinity"_s,
        //: CLI option description
        //% "Set CPU affinity with the number of CPUs available for each worker core."
        qtTrId("cutelystd-opt-cpu-affinity-desc"),
        //: CLI option value name
        //% "core count"
        qtTrId("cutelystd-opt-cpu-affinity-value"));
    parser.addOption(cpuAffinityOpt);
#endif // Q_OS_UNIX

#ifdef Q_OS_LINUX
    QCommandLineOption reusePortOpt(u"reuse-port"_s,
                                    //: CLI option description
                                    //% "Enable SO_REUSEPORT flag on socket (Linux 3.9+)."
                                    qtTrId("cutelystd-opt-reuse-port-desc"));
    parser.addOption(reusePortOpt);
#endif

    QCommandLineOption threadBalancerOpt(
        u"experimental-thread-balancer"_s,
        //: CLI option description
        //% "Balances new connections to threads using round-robin."
        qtTrId("cutelystd-opt-experimental-thread-balancer-desc"));
    parser.addOption(threadBalancerOpt);

    QCommandLineOption frontendProxy(u"using-frontend-proxy"_s,
                                     //: CLI option description
                                     //% "Enable frontend (reverse-)proxy support."
                                     qtTrId("cutelystd-opt-using-frontend-proxy-desc"));
    parser.addOption(frontendProxy);

    // Process the actual command line arguments given by the user
    parser.process(arguments);

    setIni(parser.values(iniOpt));

    setJson(parser.values(jsonOpt));

    if (parser.isSet(chdirOpt)) {
        setChdir(parser.value(chdirOpt));
    }

    if (parser.isSet(chdir2Opt)) {
        setChdir2(parser.value(chdir2Opt));
    }

    if (parser.isSet(threadsOpt)) {
        setThreads(parser.value(threadsOpt));
    }

    if (parser.isSet(socketAccessOpt)) {
        setSocketAccess(parser.value(socketAccessOpt));
    }

    if (parser.isSet(socketTimeoutOpt)) {
        bool ok;
        auto size = parser.value(socketTimeoutOpt).toInt(&ok);
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
    if (parser.isSet(stopOpt)) {
        UnixFork::stopSERVER(parser.value(stopOpt));
    }

    if (parser.isSet(processesOpt)) {
        setProcesses(parser.value(processesOpt));
    }

    if (parser.isSet(uidOpt)) {
        setUid(parser.value(uidOpt));
    }

    if (parser.isSet(gidOpt)) {
        setGid(parser.value(gidOpt));
    }

    if (parser.isSet(noInitgroupsOpt)) {
        setNoInitgroups(true);
    }

    if (parser.isSet(chownSocketOpt)) {
        setChownSocket(parser.value(chownSocketOpt));
    }

    if (parser.isSet(umaskOpt)) {
        setUmask(parser.value(umaskOpt));
    }

    if (parser.isSet(cpuAffinityOpt)) {
        bool ok;
        auto value = parser.value(cpuAffinityOpt).toInt(&ok);
        setCpuAffinity(value);
        if (!ok || value < 0) {
            parser.showHelp(1);
        }
    }
#endif // Q_OS_UNIX

#ifdef Q_OS_LINUX
    if (parser.isSet(reusePortOpt)) {
        setReusePort(true);
    }
#endif

    if (parser.isSet(lazyOpt)) {
        setLazy(true);
    }

    if (parser.isSet(listenQueueOpt)) {
        bool ok;
        auto size = parser.value(listenQueueOpt).toInt(&ok);
        setListenQueue(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(bufferSizeOpt)) {
        bool ok;
        auto size = parser.value(bufferSizeOpt).toInt(&ok);
        setBufferSize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBufferingOpt)) {
        bool ok;
        auto size = parser.value(postBufferingOpt).toLongLong(&ok);
        setPostBuffering(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBufferingBufsizeOpt)) {
        bool ok;
        auto size = parser.value(postBufferingBufsizeOpt).toLongLong(&ok);
        setPostBufferingBufsize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(applicationOpt)) {
        setApplication(parser.value(applicationOpt));
    }

    if (parser.isSet(masterOpt)) {
        setMaster(true);
    }

    if (parser.isSet(autoReloadOpt)) {
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

    if (parser.isSet(socketSndbufOpt)) {
        bool ok;
        auto size = parser.value(socketSndbufOpt).toInt(&ok);
        setSocketSndbuf(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(socketRcvbufOpt)) {
        bool ok;
        auto size = parser.value(socketRcvbufOpt).toInt(&ok);
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
    std::cout << "Cutelyst-Server starting" << '\n';

    if (!qEnvironmentVariableIsSet("CUTELYST_SERVER_IGNORE_MASTER") && !d->master) {
        std::cout
            << "*** WARNING: you are running Cutelyst-Server without its master process manager ***"
            << '\n';
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

    connect(
        d->genericFork, &AbstractFork::forked, d, &ServerPrivate::postFork, Qt::DirectConnection);
    connect(
        d->genericFork, &AbstractFork::shutdown, d, &ServerPrivate::shutdown, Qt::DirectConnection);

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
        connect(d, &ServerPrivate::postForked, sd, [sd] { sd->setWatchdog(false); });
        qInfo(CUTELYST_SERVER) << "systemd notify detected";
    }
#endif

    // TCP needs root privileges, but SO_REUSEPORT must have an effective user ID that
    // matches the effective user ID used to perform the first bind on the socket.

    if (!d->reusePort) {
        if (!d->listenTcpSockets()) {
            //% "No specified sockets were able to be opened"
            Q_EMIT errorOccured(qtTrId("cutelystd-err-no-socket-opened"));
            return 1; // No sockets has been opened
        }
    }

    if (!d->writePidFile(d->pidfile)) {
        //% "Failed to write pidfile %1"
        Q_EMIT errorOccured(qtTrId("cutelystd-err-write-pidfile").arg(d->pidfile));
    }

#ifdef Q_OS_UNIX
    bool isListeningLocalSockets = false;
    if (!d->chownSocket.isEmpty()) {
        if (!d->listenLocalSockets()) {
            //% "Error on opening local sockets"
            Q_EMIT errorOccured(qtTrId("cutelystd-err-open-local-socket"));
            return 1;
        }
        isListeningLocalSockets = true;
    }

    if (!d->umask.isEmpty() && !UnixFork::setUmask(d->umask.toLatin1())) {
        return 1;
    }

    if (!UnixFork::setGidUid(d->gid, d->uid, d->noInitgroups)) {
        //% "Error on setting GID or UID"
        Q_EMIT errorOccured(qtTrId("cutelystd-err-setgiduid"));
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
            Q_EMIT errorOccured(qtTrId("cutelystd-err-no-socket-opened"));
            return 1; // No sockets has been opened
        }
    }

    if (d->servers.empty()) {
        std::cout << "Please specify a socket to listen to" << '\n';
        //% "No socket specified"
        Q_EMIT errorOccured(qtTrId("cutelystd-err-no-socket-specified"));
        return 1;
    }

    d->writePidFile(d->pidfile2);

    if (!d->chdir.isEmpty()) {
        std::cout << "Changing directory to: " << d->chdir.toLatin1().constData() << '\n';
        if (!QDir::setCurrent(d->chdir)) {
            Q_EMIT errorOccured(QString::fromLatin1("Failed to chdir to: '%s'")
                                    .arg(QString::fromLatin1(d->chdir.toLatin1().constData())));
            return 1;
        }
    }

    d->app = app;

    if (!d->lazy) {
        if (!d->setupApplication()) {
            //% "Failed to setup Application"
            Q_EMIT errorOccured(qtTrId("cutelystd-err-fail-setup-app"));
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

    if (d->mainEngine) {
        //% "Server not fully stopped."
        Q_EMIT errorOccured(qtTrId("cutelystd-err-server-not-fully-stopped"));
        return false;
    }

    d->processes     = 0;
    d->master        = false;
    d->lazy          = false;
    d->userEventLoop = true;
#ifdef Q_OS_UNIX
    d->uid.clear();
    d->gid.clear();
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
    if (httpSockets.isEmpty() && httpsSockets.isEmpty() && http2Sockets.isEmpty() &&
        fastcgiSockets.isEmpty()) {
        // no sockets to listen to
        return false;
    }

    // HTTP
    bool httpOk = std::ranges::all_of(httpSockets, [this](const auto &socket) {
        return listenTcp(socket, getHttpProto(), false);
    });
    if (!httpOk) {
        return false;
    }

    // HTTPS
    bool httpsOk = std::ranges::all_of(httpsSockets, [this](const auto &socket) {
        return listenTcp(socket, getHttpProto(), true);
    });
    if (!httpsOk) {
        return false;
    }

    // HTTP/2
    bool http2Ok = std::ranges::all_of(http2Sockets, [this](const auto &socket) {
        return listenTcp(socket, getHttp2Proto(), false);
    });
    if (!http2Ok) {
        return false;
    }

    // FastCGI
    bool allOk = std::ranges::all_of(fastcgiSockets, [this](const QString &socket) {
        return listenTcp(socket, getFastCgiProto(), false);
    });

    return allOk;
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
            auto qEnum = Protocol::staticMetaObject.enumerator(0);
            std::cout << qEnum.valueToKey(static_cast<int>(protocol->type())) << " socket "
                      << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to TCP address " << server->serverName().constData() << " fd "
                      << QByteArray::number(server->socketDescriptor()).constData() << '\n';
            servers.emplace_back(server);
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
                std::cerr << "systemd activated socket does not match any configured socket"
                          << '\n';
                return false;
            }
            server->setProtocol(protocol);
            server->pauseAccepting();

            auto qEnum = Protocol::staticMetaObject.enumerator(0);
            std::cout << qEnum.valueToKey(static_cast<int>(protocol->type())) << " socket "
                      << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to LOCAL address " << qPrintable(fullName) << " fd "
                      << QByteArray::number(server->socket()).constData() << '\n';
            servers.push_back(server);
        } else {
            std::cerr << "Failed to listen on activated LOCAL FD: "
                      << QByteArray::number(fd).constData() << " : "
                      << qPrintable(server->errorString()) << '\n';
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
    if (line.startsWith(u'/')) {
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

        QLocalServer::removeServer(line);
        server->setListenBacklogSize(listenQueue);
        ret = server->listen(line);
        server->pauseAccepting();

        if (!ret || !server->socket()) {
            std::cerr << "Failed to listen on LOCAL: " << qPrintable(line) << " : "
                      << qPrintable(server->errorString()) << '\n';
            return false;
        }

#ifdef Q_OS_UNIX
        if (!chownSocket.isEmpty()) {
            UnixFork::chownSocket(line, chownSocket);
        }
#endif
        auto qEnum = Protocol::staticMetaObject.enumerator(0);
        std::cout << qEnum.valueToKey(static_cast<int>(protocol->type())) << " socket "
                  << QByteArray::number(static_cast<int>(servers.size())).constData()
                  << " bound to LOCAL address " << qPrintable(line) << " fd "
                  << QByteArray::number(server->socket()).constData() << '\n';
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
        return u"auto"_s;
    }
    return QString::number(d->threads);
}

void Server::setProcesses(const QString &process)
{
#ifdef Q_OS_UNIX
    Q_D(Server);
    if (process.compare(u"auto", Qt::CaseInsensitive) == 0) {
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
        return u"auto"_s;
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

    for (const QString &file : files) {
        if (!d->configLoaded.contains(file)) {
            auto fileToLoad = std::make_pair(file, ServerPrivate::ConfigFormat::Ini);
            if (!d->configToLoad.contains(fileToLoad)) {
                qCDebug(CUTELYST_SERVER) << "Enqueue INI config file:" << file;
                d->configToLoad.enqueue(fileToLoad);
            }
        }
    }

    d->loadConfig();
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

    for (const QString &file : files) {
        if (!d->configLoaded.contains(file)) {
            auto fileToLoad = std::make_pair(file, ServerPrivate::ConfigFormat::Json);
            if (!d->configToLoad.contains(fileToLoad)) {
                qCDebug(CUTELYST_SERVER) << "Enqueue JSON config file:" << file;
                d->configToLoad.enqueue(fileToLoad);
            }
        }
    }

    d->loadConfig();
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
#else
    Q_UNUSED(enable);
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

QVariantMap Server::config() const noexcept
{
    Q_D(const Server);
    return d->config;
}

bool ServerPrivate::setupApplication()
{
    Cutelyst::Application *localApp = app;

    Q_Q(Server);

    if (!localApp) {
        std::cout << "Loading application: " << application.toLatin1().constData() << '\n';
        QPluginLoader loader(application);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::PreventUnloadHint);
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
            qCCritical(CUTELYST_SERVER)
                << "Could not cast Cutelyst::Application from instance: %s\n"
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
        std::cout << "Changing directory2 to: " << chdir2.toLatin1().constData() << '\n';
        if (!QDir::setCurrent(chdir2)) {
            Q_EMIT q->errorOccured(QString::fromLatin1("Failed to chdir2 to: '%s'")
                                       .arg(QString::fromLatin1(chdir2.toLatin1().constData())));
            return false;
        }
    }

    if (threads > 1) {
        mainEngine = createEngine(localApp, 0);
        for (int i = 1; i < threads; ++i) {
            if (createEngine(localApp, i)) {
                ++workersNotRunning;
            }
        }
    } else {
        mainEngine        = createEngine(localApp, 0);
        workersNotRunning = 1;
    }

    if (!mainEngine) {
        std::cerr << "Application failed to init, cheaping..." << '\n';
        return false;
    }

    return true;
}

void ServerPrivate::engineShutdown(ServerEngine *engine)
{
    const auto engineThread = engine->thread();
    if (QThread::currentThread() != engineThread) {
        connect(engineThread, &QThread::finished, this, [this, engine] {
            auto [first, last] = std::ranges::remove(engines, engine);
            engines.erase(first, last);
            checkEngineShutdown();
        });
        engineThread->quit();
    } else {
        auto [first, last] = std::ranges::remove(engines, engine);
        engines.erase(first, last);
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
            QTimer::singleShot(std::chrono::seconds{0}, this, [] { qApp->exit(15); });
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
            Q_EMIT q->errorOccured(qtTrId("cutelystd-err-fail-setup-app"));
            return false;
        }
    }

    if (engines.size() > 1) {
        qCDebug(CUTELYST_SERVER) << "Starting threads";
    }

    for (ServerEngine *engine : engines) {
        QThread *thread = engine->thread();
        if (thread != qApp->thread()) {
#ifdef Q_OS_LINUX
            if (!qEnvironmentVariableIsSet("CUTELYST_QT_EVENT_LOOP")) {
                // NOLINTNEXTLINE
                thread->setEventDispatcher(new EventDispatcherEPoll);
            }
#endif

            thread->start();
        }
    }

    Q_EMIT postForked(workerId);

    QTimer::singleShot(std::chrono::seconds{1}, this, [=]() {
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
        std::cerr << "Failed write pid file " << qPrintable(filename) << '\n';
        return false;
    }

    std::cout << "Writing pidfile to " << qPrintable(filename) << '\n';
    file.write(QByteArray::number(QCoreApplication::applicationPid()) + '\n');

    return true;
}

ServerEngine *ServerPrivate::createEngine(Application *app, int workerCore)
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

    auto engine = new ServerEngine(app, workerCore, opt, q);
    connect(this, &ServerPrivate::shutdown, engine, &ServerEngine::shutdown, Qt::QueuedConnection);
    connect(
        this, &ServerPrivate::postForked, engine, &ServerEngine::postFork, Qt::QueuedConnection);
    connect(engine,
            &ServerEngine::shutdownCompleted,
            this,
            &ServerPrivate::engineShutdown,
            Qt::QueuedConnection);
    connect(
        engine, &ServerEngine::started, this, &ServerPrivate::workerStarted, Qt::QueuedConnection);

    engine->setConfig(config);
    engine->setServers(servers);
    if (!engine->init()) {
        std::cerr << "Application failed to init(), cheaping core: " << workerCore << '\n';
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

void ServerPrivate::loadConfig()
{
    if (loadingConfig) {
        return;
    }

    loadingConfig = true;

    if (configToLoad.isEmpty()) {
        loadingConfig = false;
        return;
    }

    auto fileToLoad = configToLoad.dequeue();

    if (fileToLoad.first.isEmpty()) {
        qCWarning(CUTELYST_SERVER) << "Can not load config from empty config file name";
        loadingConfig = false;
        return;
    }

    if (configLoaded.contains(fileToLoad.first)) {
        loadingConfig = false;
        return;
    }

    configLoaded.append(fileToLoad.first);

    QVariantMap loadedConfig;
    switch (fileToLoad.second) {
    case ConfigFormat::Ini:
        qCInfo(CUTELYST_SERVER) << "Loading INI configuratin:" << fileToLoad.first;
        loadedConfig = Engine::loadIniConfig(fileToLoad.first);
        break;
    case ConfigFormat::Json:
        qCInfo(CUTELYST_SERVER) << "Loading JSON configuration:" << fileToLoad.first;
        loadedConfig = Engine::loadJsonConfig(fileToLoad.first);
        break;
    }

    for (const auto &[key, value] : std::as_const(loadedConfig).asKeyValueRange()) {
        if (config.contains(key)) {
            QVariantMap currentMap      = config.value(key).toMap();
            const QVariantMap loadedMap = value.toMap();
            for (const auto &[mapKey, mapValue] : loadedMap.asKeyValueRange()) {
                currentMap.insert(mapKey, mapValue);
            }
            config.insert(key, currentMap);
        } else {
            config.insert(key, value);
        }
    }

    QVariantMap sessionConfig = loadedConfig.value(u"server"_s).toMap();

    applyConfig(sessionConfig);

    opt.insert(sessionConfig);

    loadingConfig = false;

    if (!configToLoad.empty()) {
        loadConfig();
    }
}

void ServerPrivate::applyConfig(const QVariantMap &config)
{
    Q_Q(Server);

    for (const auto &[key, value] : config.asKeyValueRange()) {
        QString normKey = key;
        normKey.replace(u'-', u'_');

        int ix = q->metaObject()->indexOfProperty(normKey.toLatin1().constData());
        if (ix == -1) {
            continue;
        }

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
