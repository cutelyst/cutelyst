/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "wsgi_p.h"

#include "protocol.h"
#include "protocolhttp.h"
#include "protocolhttp2.h"
#include "protocolfastcgi.h"
#include "cwsgiengine.h"
#include "socket.h"
#include "tcpserverbalancer.h"
#include "tcpsslserver.h"
#include "localserver.h"

#ifdef Q_OS_UNIX
#include "unixfork.h"
#else
#include "windowsfork.h"
#endif

#ifdef Q_OS_LINUX
#include "../EventLoopEPoll/eventdispatcher_epoll.h"
#include "systemdnotify.h"
#endif

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QPluginLoader>
#include <QThread>
#include <QLoggingCategory>
#include <QSettings>
#include <QSocketNotifier>
#include <QMetaProperty>
#include <QTimer>
#include <QDir>
#include <QMutex>

#include <iostream>

Q_LOGGING_CATEGORY(CUTELYST_WSGI, "wsgi")

using namespace CWSGI;
using namespace Cutelyst;

WSGI::WSGI(QObject *parent) : QObject(parent),
    d_ptr(new WSGIPrivate(this))
{
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qSetMessagePattern(QStringLiteral("%{pid}:%{threadid} %{category}[%{type}] %{message}"));
    }

#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsSet("CUTELYST_EVENT_LOOP_EPOLL")) {
        std::cout << "Installing EPoll event loop" << std::endl;
        QCoreApplication::setEventDispatcher(new EventDispatcherEPoll);
    }
#endif
}

WSGI::~WSGI()
{
    Q_D(WSGI);

    delete d->protoHTTP;
    delete d->protoFCGI;

    std::cout << "Cutelyst-WSGI terminated" << std::endl;
}

void WSGI::parseCommandLine(const QStringList &arguments)
{
    Q_D(WSGI);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Fast, developer-friendly WSGI server"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniOpt(QStringLiteral("ini"),
                              QCoreApplication::translate("main", "load config from ini file"),
                              QCoreApplication::translate("main", "file"));
    parser.addOption(iniOpt);

    QCommandLineOption jsonOpt({ QStringLiteral("j"), QStringLiteral("json") },
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

    QCommandLineOption application({ QStringLiteral("application"), QStringLiteral("a") },
                                   QCoreApplication::translate("main", "Application to load"),
                                   QCoreApplication::translate("main", "file"));
    parser.addOption(application);

    QCommandLineOption threads({ QStringLiteral("threads"), QStringLiteral("t") },
                               QCoreApplication::translate("main", "Number of thread to use"),
                               QCoreApplication::translate("main", "threads"));
    parser.addOption(threads);

#ifdef Q_OS_UNIX
    QCommandLineOption processes({ QStringLiteral("processes"), QStringLiteral("p") },
                                 QCoreApplication::translate("main", "spawn the specified number of processes"),
                                 QCoreApplication::translate("main", "processes"));
    parser.addOption(processes);
#endif

    QCommandLineOption master({ QStringLiteral("master"), QStringLiteral("M") },
                              QCoreApplication::translate("main", "Enable master process"));
    parser.addOption(master);

    QCommandLineOption bufferSize({ QStringLiteral("buffer-size"), QStringLiteral("b") },
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

    QCommandLineOption httpSocketOpt({ QStringLiteral("http-socket"), QStringLiteral("h1") },
                                     QCoreApplication::translate("main", "bind to the specified TCP socket using HTTP protocol"),
                                     QCoreApplication::translate("main", "address"));
    parser.addOption(httpSocketOpt);

    QCommandLineOption http2SocketOpt({ QStringLiteral("http2-socket"), QStringLiteral("h2") },
                                     QCoreApplication::translate("main", "bind to the specified TCP socket using HTTP/2 protocol"),
                                     QCoreApplication::translate("main", "address"));
    parser.addOption(http2SocketOpt);

    QCommandLineOption http2HeaderTableSizeOpt({ QStringLiteral("http2-header-table-size") },
                                               QCoreApplication::translate("main", "Defined the HTTP/2 header table size"),
                                               QCoreApplication::translate("main", "size"));
    parser.addOption(http2HeaderTableSizeOpt);

    QCommandLineOption httpsSocketOpt({ QStringLiteral("https-socket"), QStringLiteral("hs1") },
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

    QCommandLineOption socketTimeout({ QStringLiteral("socket-timeout"), QStringLiteral("z") },
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

    QCommandLineOption autoReload({ QStringLiteral("auto-restart"), QStringLiteral("r") },
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

    // Process the actual command line arguments given by the user
    parser.process(arguments);

    const auto inis = parser.values(iniOpt);
    for (const QString &ini : inis) {
        d->loadConfig(ini, false);
    }
    setIni(inis);

    const auto jsons = parser.values(jsonOpt);
    for (const QString &json : jsons) {
        d->loadConfig(json, true);
    }
    setJson(jsons);

    d->applyConfig(d->opt);

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

    if (parser.isSet(bufferSize)) {
        bool ok;
        auto size = parser.value(bufferSize).toLongLong(&ok);
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

    setHttpSocket(httpSocket() + parser.values(httpSocketOpt));

    setHttp2Socket(http2Socket() + parser.values(http2SocketOpt));

    setHttpsSocket(httpsSocket() + parser.values(httpsSocketOpt));

    setFastcgiSocket(fastcgiSocket() + parser.values(fastcgiSocketOpt));

    setStaticMap(staticMap() + parser.values(staticMapOpt));

    setStaticMap2(staticMap2() + parser.values(staticMap2Opt));

    setTouchReload(touchReload() + parser.values(touchReloadOpt));

    d->threadBalancer = parser.isSet(threadBalancerOpt);
}

int WSGI::exec(Cutelyst::Application *app)
{
    Q_D(WSGI);
    std::cout << "Cutelyst-WSGI starting" << std::endl;

    if (!qEnvironmentVariableIsSet("CUTELYST_WSGI_IGNORE_MASTER") && !d->master) {
        std::cout << "*** WARNING: you are running Cutelyst-WSGI without its master process manager ***" << std::endl;
    }

#ifdef Q_OS_UNIX
    if (d->processes == -1 && d->threads == -1) {
        d->processes = UnixFork::idealProcessCount();
        d->threads = UnixFork::idealThreadCount() / d->processes;
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

    connect(d->genericFork, &AbstractFork::forked, d, &WSGIPrivate::postFork, Qt::DirectConnection);
    connect(d->genericFork, &AbstractFork::shutdown, d, &WSGIPrivate::shutdown, Qt::DirectConnection);

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
    systemdNotify::install_systemd_notifier(this);
#endif

    // TCP needs root privileges
    d->listenTcpSockets();

    d->writePidFile(d->pidfile);

    bool isListeningLocalSockets = false;
#ifdef Q_OS_UNIX
    if (!d->chownSocket.isEmpty()) {
        d->listenLocalSockets();
        isListeningLocalSockets = true;
    }

    if (!d->umask.isEmpty() &&
            !UnixFork::setUmask(d->umask)) {
        return 1;
    }

    UnixFork::setGidUid(d->gid, d->uid, d->noInitgroups);
#endif

    if (!isListeningLocalSockets) {
        d->listenLocalSockets();
    }

    if (!d->servers.size()) {
        std::cout << "Please specify a socket to listen to" << std::endl;
        return 1;
    }

    d->writePidFile(d->pidfile2);

    if (!d->chdir.isEmpty()) {
        std::cout << "Changing directory to: " << d->chdir.toLatin1().constData() << std::endl;;
        if (!QDir::setCurrent(d->chdir)) {
            qFatal("Failed to chdir to: '%s'", d->chdir.toLatin1().constData());
        }
    }

    d->app = app;

    if (!d->lazy) {
        d->setupApplication();
    }

    if (d->userEventLoop) {
        d->postFork(0);
        return 0;
    }

    ret = d->genericFork->exec(d->lazy, d->master);

    return ret;
}

bool WSGI::start(Application *app)
{
    Q_D(WSGI);

    d->processes = 0;
    d->master = false;
    d->lazy = false;
    d->userEventLoop = true;
#ifdef Q_OS_UNIX
    d->uid = QString();
    d->gid = QString();
#endif
    qputenv("CUTELYST_WSGI_IGNORE_MASTER", QByteArrayLiteral("1"));

    if (exec(app) == 0) {
        return true;
    }

    return false;
}

void WSGI::stop()
{
    Q_D(WSGI);
    if (d->userEventLoop) {
        Q_EMIT d->shutdown();
    }
}

void WSGIPrivate::listenTcpSockets()
{
    Q_Q(WSGI);

    if (!httpSockets.isEmpty()) {
        if (!protoHTTP) {
            protoHTTP = new ProtocolHttp(q);
        }

        const auto sockets = httpSockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoHTTP, false);
        }
    }

    if (!http2Sockets.isEmpty()) {
        if (!protoHTTP2) {
            protoHTTP2 = new ProtocolHttp2(q);
        }

        const auto sockets = http2Sockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoHTTP2, false);
        }
    }

    if (!httpsSockets.isEmpty()) {
        if (!protoHTTP) {
            protoHTTP = new ProtocolHttp(q);
        }

        const auto sockets = httpsSockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoHTTP, true);
        }
    }

    if (!fastcgiSockets.isEmpty()) {
        if (!protoFCGI) {
            protoFCGI = new ProtocolFastCGI(q);
        }

        const auto sockets = fastcgiSockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoFCGI, false);
        }
    }
}

bool WSGIPrivate::listenTcp(const QString &line, Protocol *protocol, bool secure)
{
    Q_Q(WSGI);

    bool ret = true;
    if (!line.startsWith(QLatin1Char('/'))) {
        auto server = new TcpServerBalancer(q);
        server->setBalancer(threadBalancer);
        ret = server->listen(line, protocol, secure);

        if (ret && server->socketDescriptor()) {
            std::cout << "WSGI socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to TCP address " << qPrintable(server->serverName())
                      << " fd " << QByteArray::number(server->socketDescriptor()).constData()
                      << std::endl;
            servers.push_back(server);
        }
    }

    return ret;
}

void WSGIPrivate::listenLocalSockets()
{
    Q_Q(WSGI);

    QStringList http = httpsSockets;
    QStringList fastcgi = fastcgiSockets;

    if (!http.isEmpty() && !protoHTTP) {
        protoHTTP = new ProtocolHttp(q);
    }

    if (!fastcgi.isEmpty() && !protoFCGI) {
        protoFCGI = new ProtocolFastCGI(q);
    }

#ifdef Q_OS_LINUX
    std::vector<int> fds = systemdNotify::listenFds();
    for (int fd : fds) {
        auto server = new LocalServer(q, this);
        if (server->listen(fd)) {
            const QString name = server->serverName();
            const QString fullName = server->fullServerName();

            Protocol *protocol;
            if (http.removeOne(fullName) || http.removeOne(name)) {
                protocol = protoHTTP;
            } else if (fastcgi.removeOne(fullName)  || fastcgi.removeOne(name)) {
                protocol = protoFCGI;
            } else {
                qFatal("systemd activated socket does not match any configured socket");
            }
            server->setProtocol(protocol);
            server->pauseAccepting();

            std::cout << "WSGI socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                      << " bound to LOCAL address " << qPrintable(fullName)
                      << " fd " << QByteArray::number(server->socket()).constData()
                      << std::endl;
            servers.push_back(server);
        } else {
            std::cout << "Failed to listen on activated LOCAL FD: " << QByteArray::number(fd).constData()
                      << " : " << qPrintable(server->errorString()) << std::endl;
            exit(1);
        }
    }
#endif

    const auto httpConst = http;
    for (const auto &socket : httpConst) {
        listenLocal(socket, protoHTTP);
    }

    const auto fastcgiConst = fastcgi;
    for (const auto &socket : fastcgiConst) {
        listenLocal(socket, protoFCGI);
    }
}

bool WSGIPrivate::listenLocal(const QString &line, Protocol *protocol)
{
    Q_Q(WSGI);

    bool ret = true;
    if (line.startsWith(QLatin1Char('/'))) {
        auto server = new LocalServer(q, this);
        server->setProtocol(protocol);
        if (!socketAccess.isEmpty()) {
            QLocalServer::SocketOptions options;
            if (socketAccess.contains(QLatin1Char('u'))) {
                options |= QLocalServer::UserAccessOption;
            }

            if (socketAccess.contains(QLatin1Char('g'))) {
                options |= QLocalServer::GroupAccessOption;
            }

            if (socketAccess.contains(QLatin1Char('o'))) {
                options |= QLocalServer::OtherAccessOption;
            }
            server->setSocketOptions(options);
        }
        server->removeServer(line);
        ret = server->listen(line);
        server->pauseAccepting();

        if (!ret || !server->socket()) {
            std::cout << "Failed to listen on LOCAL: " << qPrintable(line)
                      << " : " << qPrintable(server->errorString()) << std::endl;
            exit(1);
        }

#ifdef Q_OS_UNIX
        if (!chownSocket.isEmpty()) {
            UnixFork::chownSocket(line, chownSocket);
        }
#endif

        std::cout << "WSGI socket " << QByteArray::number(static_cast<int>(servers.size())).constData()
                  << " bound to LOCAL address " << qPrintable(line)
                  << " fd " << QByteArray::number(server->socket()).constData()
                  << std::endl;
        servers.push_back(server);
    }

    return ret;
}

void WSGI::setApplication(const QString &application)
{
    Q_D(WSGI);
    d->application = application;
}

QString WSGI::application() const
{
    Q_D(const WSGI);
    return d->application;
}

void WSGI::setThreads(const QString &threads)
{
    Q_D(WSGI);
    if (threads.compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0) {
        d->threads = -1;
    } else {
        d->threads = threads.toInt();
    }
}

QString WSGI::threads() const
{
    Q_D(const WSGI);
    if (d->threads == -1) {
        return QStringLiteral("auto");
    }
    return QString::number(d->threads);
}

#ifdef Q_OS_UNIX
void WSGI::setProcesses(const QString &process)
{
    Q_D(WSGI);
    if (process.compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0) {
        d->processes = -1;
    } else {
        d->processes = process.toInt();
    }
}

QString WSGI::processes() const
{
    Q_D(const WSGI);
    if (d->processes == -1) {
        return QStringLiteral("auto");
    }
    return QString::number(d->processes);
}
#endif

void WSGI::setChdir(const QString &chdir)
{
    Q_D(WSGI);
    d->chdir = chdir;
}

QString WSGI::chdir() const
{
    Q_D(const WSGI);
    return d->chdir;
}

void WSGI::setHttpSocket(const QStringList &httpSocket)
{
    Q_D(WSGI);
    d->httpSockets = httpSocket;
}

QStringList WSGI::httpSocket() const
{
    Q_D(const WSGI);
    return d->httpSockets;
}

void WSGI::setHttp2Socket(const QStringList &http2Socket)
{
    Q_D(WSGI);
    d->http2Sockets = http2Socket;
}

QStringList WSGI::http2Socket() const
{
    Q_D(const WSGI);
    return d->http2Sockets;
}

void WSGI::setHttp2HeaderTableSize(quint32 headerTableSize)
{
    Q_D(WSGI);
    d->http2HeaderTableSize = headerTableSize;
}

quint32 WSGI::http2HeaderTableSize() const
{
    Q_D(const WSGI);
    return d->http2HeaderTableSize;
}

void WSGI::setHttpsSocket(const QStringList &httpsSocket)
{
    Q_D(WSGI);
    d->httpsSockets = httpsSocket;
}

QStringList WSGI::httpsSocket() const
{
    Q_D(const WSGI);
    return d->httpsSockets;
}

void WSGI::setFastcgiSocket(const QStringList &fastcgiSocket)
{
    Q_D(WSGI);
    d->fastcgiSockets = fastcgiSocket;
}

QStringList WSGI::fastcgiSocket() const
{
    Q_D(const WSGI);
    return d->fastcgiSockets;
}

void WSGI::setSocketAccess(const QString &socketAccess)
{
    Q_D(WSGI);
    d->socketAccess = socketAccess;
}

QString WSGI::socketAccess() const
{
    Q_D(const WSGI);
    return d->socketAccess;
}

void WSGI::setSocketTimeout(int timeout)
{
    Q_D(WSGI);
    d->socketTimeout = timeout;
}

int WSGI::socketTimeout() const
{
    Q_D(const WSGI);
    return d->socketTimeout;
}

void WSGI::setChdir2(const QString &chdir2)
{
    Q_D(WSGI);
    d->chdir2 = chdir2;
}

QString WSGI::chdir2() const
{
    Q_D(const WSGI);
    return d->chdir2;
}

void WSGI::setIni(const QStringList &ini)
{
    Q_D(WSGI);
    d->ini = ini;
}

QStringList WSGI::ini() const
{
    Q_D(const WSGI);
    return d->ini;
}

void WSGI::setJson(const QStringList &files)
{
    Q_D(WSGI);
    d->json = files;
}

QStringList WSGI::json() const
{
    Q_D(const WSGI);
    return d->json;
}

void WSGI::setStaticMap(const QStringList &staticMap)
{
    Q_D(WSGI);
    d->staticMaps = staticMap;
}

QStringList WSGI::staticMap() const
{
    Q_D(const WSGI);
    return d->staticMaps;
}

void WSGI::setStaticMap2(const QStringList &staticMap)
{
    Q_D(WSGI);
    d->staticMaps2 = staticMap;
}

QStringList WSGI::staticMap2() const
{
    Q_D(const WSGI);
    return d->staticMaps2;
}

void WSGI::setMaster(bool enable)
{
    Q_D(WSGI);
    if (!qEnvironmentVariableIsSet("CUTELYST_WSGI_IGNORE_MASTER")) {
        d->master = enable;
    }
}

bool WSGI::master() const
{
    Q_D(const WSGI);
    return d->master;
}

void WSGI::setAutoReload(bool enable)
{
    Q_D(WSGI);
    if (enable) {
        d->autoReload = true;
    }
}

bool WSGI::autoReload() const
{
    Q_D(const WSGI);
    return d->autoReload;
}

void WSGI::setTouchReload(const QStringList &files)
{
    Q_D(WSGI);
    d->touchReload = files;
}

QStringList WSGI::touchReload() const
{
    Q_D(const WSGI);
    return d->touchReload;
}

void WSGI::setBufferSize(qint64 size)
{
    Q_D(WSGI);
    if (size < 4096) {
        qCWarning(CUTELYST_WSGI) << "Buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    d->bufferSize = size;
}

int WSGI::bufferSize() const
{
    Q_D(const WSGI);
    return d->bufferSize;
}

void WSGI::setPostBuffering(qint64 size)
{
    Q_D(WSGI);
    d->postBuffering = size;
}

qint64 WSGI::postBuffering() const
{
    Q_D(const WSGI);
    return d->postBuffering;
}

void WSGI::setPostBufferingBufsize(qint64 size)
{
    Q_D(WSGI);
    if (size < 4096) {
        qCWarning(CUTELYST_WSGI) << "Post buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    d->postBufferingBufsize = size;
}

qint64 WSGI::postBufferingBufsize() const
{
    Q_D(const WSGI);
    return d->postBufferingBufsize;
}

void WSGI::setTcpNodelay(bool enable)
{
    Q_D(WSGI);
    d->tcpNodelay = enable;
}

bool WSGI::tcpNodelay() const
{
    Q_D(const WSGI);
    return d->tcpNodelay;
}

void WSGI::setSoKeepalive(bool enable)
{
    Q_D(WSGI);
    d->soKeepalive = enable;
}

bool WSGI::soKeepalive() const
{
    Q_D(const WSGI);
    return d->soKeepalive;
}

void WSGI::setSocketSndbuf(int value)
{
    Q_D(WSGI);
    d->socketSendBuf = value;
}

int WSGI::socketSndbuf() const
{
    Q_D(const WSGI);
    return d->socketSendBuf;
}

void WSGI::setSocketRcvbuf(int value)
{
    Q_D(WSGI);
    d->socketReceiveBuf = value;
}

int WSGI::socketRcvbuf() const
{
    Q_D(const WSGI);
    return d->socketReceiveBuf;
}

void WSGI::setWebsocketMaxSize(int value)
{
    Q_D(WSGI);
    d->websocketMaxSize = value * 1024;
}

int WSGI::websocketMaxSize() const
{
    Q_D(const WSGI);
    return d->websocketMaxSize / 1024;
}

void WSGI::setPidfile(const QString &file)
{
    Q_D(WSGI);
    d->pidfile = file;
}

QString WSGI::pidfile() const
{
    Q_D(const WSGI);
    return d->pidfile;
}

void WSGI::setPidfile2(const QString &file)
{
    Q_D(WSGI);
    d->pidfile2 = file;
}

QString WSGI::pidfile2() const
{
    Q_D(const WSGI);
    return d->pidfile2;
}

#ifdef Q_OS_UNIX
void WSGI::setUid(const QString &uid)
{
    Q_D(WSGI);
    d->uid = uid;
}

QString WSGI::uid() const
{
    Q_D(const WSGI);
    return d->uid;
}

void WSGI::setGid(const QString &gid)
{
    Q_D(WSGI);
    d->gid = gid;
}

QString WSGI::gid() const
{
    Q_D(const WSGI);
    return d->gid;
}

void WSGI::setNoInitgroups(bool enable)
{
    Q_D(WSGI);
    d->noInitgroups = enable;
}

bool WSGI::noInitgroups() const
{
    Q_D(const WSGI);
    return d->noInitgroups;
}

void WSGI::setChownSocket(const QString &chownSocket)
{
    Q_D(WSGI);
    d->chownSocket = chownSocket;
}

QString WSGI::chownSocket() const
{
    Q_D(const WSGI);
    return d->chownSocket;
}

void WSGI::setUmask(const QString &value)
{
    Q_D(WSGI);
    d->umask = value;
}

QString WSGI::umask() const
{
    Q_D(const WSGI);
    return d->umask;
}

void WSGI::setCpuAffinity(int value)
{
    Q_D(WSGI);
    d->cpuAffinity = value;
}

int WSGI::cpuAffinity() const
{
    Q_D(const WSGI);
    return d->cpuAffinity;
}
#endif

#ifdef Q_OS_LINUX
void WSGI::setReusePort(bool enable)
{
    Q_D(WSGI);
    d->reusePort = enable;
}

bool WSGI::reusePort() const
{
    Q_D(const WSGI);
    return d->reusePort;
}
#endif

void WSGI::setLazy(bool enable)
{
    Q_D(WSGI);
    d->lazy = enable;
}

bool WSGI::lazy() const
{
    Q_D(const WSGI);
    return d->lazy;
}

void WSGIPrivate::setupApplication()
{
    Cutelyst::Application *localApp = app;
    if (!localApp) {
        std::cout << "Loading application: " << application.toLatin1().constData() << std::endl;;
        QPluginLoader loader(application);
        if (!loader.load()) {
            qCCritical(CUTELYST_WSGI) << "Could not load application:" << loader.errorString();
            exit(1);
        }

        QObject *instance = loader.instance();
        if (!instance) {
            qCCritical(CUTELYST_WSGI) << "Could not get a QObject instance: %s\n" << loader.errorString();
            exit(1);
        }

        localApp = qobject_cast<Cutelyst::Application *>(instance);
        if (!localApp) {
            qCCritical(CUTELYST_WSGI) << "Could not cast Cutelyst::Application from instance: %s\n" << loader.errorString();
            exit(1);
        }

        // Sets the application name with the name from our library
        //    if (QCoreApplication::applicationName() == applicationName) {
        //        QCoreApplication::setApplicationName(QString::fromLatin1(app->metaObject()->className()));
        //    }
        qCDebug(CUTELYST_WSGI) << "Loaded application: " << QCoreApplication::applicationName();
    }

    if (!chdir2.isEmpty()) {
        std::cout << "Changing directory2 to: " << chdir2.toLatin1().constData()  << std::endl;;
        if (!QDir::setCurrent(chdir2)) {
            qFatal("Failed to chdir2 to: '%s'", chdir2.toLatin1().constData());
        }
    }

    std::cout << "Threads:" << threads << std::endl;
    if (threads) {
        engine = createEngine(localApp, 0);
        for (int i = 1; i < threads; ++i) {
            if (createEngine(localApp, i)) {
                ++workersNotRunning;
            }
        }
    } else {
        engine = createEngine(localApp, 0);
    }

    if (!engine) {
        qFatal("Main engine failed to init");
    }
}

void WSGIPrivate::engineShutdown(CWsgiEngine *engine)
{
    engines.erase(std::remove(engines.begin(), engines.end(), engine), engines.end());

    const auto engineThread = engine->thread();
    if (QThread::currentThread() != engineThread) {
        engineThread->quit();
        engineThread->wait(30 * 1000);
    }

    if (engines.empty()) {
        if (userEventLoop) {
            Q_Q(WSGI);
            Q_EMIT q->stopped();
        } else {
            QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        }
    }
}

void WSGIPrivate::workerStarted()
{
    Q_Q(WSGI);

    // All workers have started
    if (--workersNotRunning == 0) {
        Q_EMIT q->ready();
    }
}

void WSGIPrivate::postFork(int workerId)
{
    if (lazy) {
        setupApplication();
    }

    if (engines.size() > 1) {
        qCDebug(CUTELYST_WSGI) << "Starting threads";
    }

    for (CWsgiEngine *engine : engines) {
        QThread *thread = engine->thread();
        if (thread != qApp->thread()) {
#ifdef Q_OS_LINUX
            if (qEnvironmentVariableIsSet("CUTELYST_EVENT_LOOP_EPOLL")) {
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
}

void WSGIPrivate::writePidFile(const QString &filename)
{
    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Failed write pid file " << qPrintable(filename) << std::endl;
        exit(1);
    }

    std::cout << "Writting pidfile to " << qPrintable(filename) << std::endl;
    file.write(QByteArray::number(QCoreApplication::applicationPid()) + '\n');
}

CWsgiEngine *WSGIPrivate::createEngine(Application *app, int core)
{
    Q_Q(WSGI);

    auto engine = new CWsgiEngine(app, core, opt, q);
    connect(this, &WSGIPrivate::shutdown, engine, &CWsgiEngine::shutdown, Qt::QueuedConnection);
    connect(this, &WSGIPrivate::postForked, engine, &CWsgiEngine::postFork, Qt::QueuedConnection);
    connect(engine, &CWsgiEngine::shutdownCompleted, this, &WSGIPrivate::engineShutdown, Qt::QueuedConnection);
    connect(engine, &CWsgiEngine::started, this, &WSGIPrivate::workerStarted, Qt::QueuedConnection);

    engine->setConfig(config);
    engine->setServers(servers);
    if (!engine->init()) {
        qCCritical(CUTELYST_WSGI) << "Failed to init engine for core:" << core;
        delete engine;
        return nullptr;
    }

    engines.push_back(engine);

    if (threads) {
        auto thread = new QThread(this);
        engine->moveToThread(thread);
    } else {
        engine->setParent(this);
    }

    return engine;
}

void WSGIPrivate::loadConfig(const QString &file, bool json)
{
    QString filename = file;

    QString section = QStringLiteral("wsgi");
    if (filename.contains(QLatin1Char(':'))) {
        section = filename.section(QLatin1Char(':'), -1, 1);
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
    sessionConfig.unite(opt);
    opt = sessionConfig;

    auto it = config.begin();
    while (it != config.end()) {
        auto itLoaded = loadedConfig.find(it.key());
        while (itLoaded == loadedConfig.end()) {
            QVariantMap loadedMap = itLoaded.value().toMap();
            loadedMap.unite(it.value().toMap());
            it.value() = loadedMap;
            loadedConfig.erase(itLoaded);
        }
        ++it;
    }

    loadedConfig.unite(config);
    config = loadedConfig;
}

void WSGIPrivate::applyConfig(const QVariantMap &config)
{
    Q_Q(WSGI);

    auto it = config.constBegin();
    while (it != config.constEnd()) {
        QString normKey = it.key();
        normKey.replace(QLatin1Char('-'), QLatin1Char('_'));

        int ix = q->metaObject()->indexOfProperty(normKey.toLatin1().constData());
        if (ix == -1) {
            ++it;
            continue;
        }

        const QVariant value = it.value();
        const QMetaProperty prop = q->metaObject()->property(ix);
        if (prop.type() == value.type()) {
            if (prop.type() == QVariant::StringList) {
                const QStringList currentValues = prop.read(q).toStringList();
                prop.write(q, currentValues + value.toStringList());
            } else {
                prop.write(q, value);
            }
        } else if (prop.type() == QVariant::StringList) {
            const QStringList currentValues = prop.read(q).toStringList();
            prop.write(q, currentValues + QStringList{ value.toString() });
        } else {
            prop.write(q, value);
        }

        ++it;
    }
}

#include "moc_wsgi.cpp"
#include "moc_wsgi_p.cpp"
