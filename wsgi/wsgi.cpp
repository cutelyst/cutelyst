/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "wsgi_p.h"

#include "protocol.h"
#include "protocolhttp.h"
#include "protocolfastcgi.h"
#include "cwsgiengine.h"
#include "socket.h"

#ifdef Q_OS_UNIX
#include "unixfork.h"
#endif

#ifdef Q_OS_LINUX
#include "../EventLoopEPoll/eventdispatcher_epoll.h"
#endif

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QHostAddress>
#include <QLocalServer>
#include <QTcpServer>
#include <QPluginLoader>
#include <QThread>
#include <QLoggingCategory>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QSocketNotifier>
#include <QTimer>
#include <QDir>

#include <iostream>

Q_LOGGING_CATEGORY(CUTELYST_WSGI, "cwsgi")

using namespace CWSGI;

WSGI::WSGI(QObject *parent) : QObject(parent),
    d_ptr(new WSGIPrivate(this))
{
    std::cout << "Cutelyst WSGI starting" << std::endl;

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

    std::cout << "Cutelyst WSGI stopping" << std::endl;
    const auto engines = d->engines;
    for (auto engine : engines) {
        engine->thread()->quit();
    }

    for (auto engine : engines) {
        engine->thread()->wait(30 * 1000);
    }

    for (auto engine : engines) {
        if (engine->thread()->isFinished()) {
            delete engine;
        }
    }

    delete d->protoHTTP;
    delete d->protoFCGI;
}

int WSGI::load(Cutelyst::Application *app)
{
    Q_D(WSGI);

    d->parseCommandLine();

    if (!d->ini.isEmpty()) {
        std::cout << "Loading configuration: " << d->ini.toLatin1().constData() << std::endl;
        if (!d->loadConfig()) {
            qCCritical(CUTELYST_WSGI) << "Failed to load config " << d->ini;
            return 1;
        }
    }

    if (d->master) {
        d->proc();
        return 0;
    }

    return d->setupApplication(app);
}

bool WSGIPrivate::listenTcp(const QString &line, Protocol *protocol)
{
    bool ret;
    SocketInfo info;
    info.protocol = protocol;

    if (line.startsWith(QLatin1Char('/'))) {
        auto server = new QLocalServer(this);
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
//        server->pauseAccepting(); // TODO

        info.serverName = line;
        info.localSocket = true;

        // THIS IS A HACK
        // QLocalServer does not expose the socket
        // descriptor, so we get it from it's QSocketNotifier child
        // if this breaks it we fail with an error.
        const auto children = server->children();
        for (auto child : children) {
            auto notifier = qobject_cast<QSocketNotifier*>(child);
            if (notifier) {
//                qDebug() << "found notifier" << notifier->socket();
                info.socketDescriptor = notifier->socket();
                notifier->deleteLater();
                break;
            }
        }
    } else {
        const QStringList parts = line.split(QLatin1Char(':'));
        if (parts.size() != 2) {
            qCDebug(CUTELYST_WSGI) << "error parsing:" << line;
            return false;
        }

        QHostAddress address;
        if (parts.first().isEmpty()) {
            address = QHostAddress::Any;
        } else {
            address.setAddress(parts.first());
        }

        bool ok;
        int port = parts.last().toInt(&ok);
        if (!ok || (port < 1 && port > 35554)) {
            port = 80;
        }

        auto server = new QTcpServer(this);
        ret = server->listen(address, port);
        server->pauseAccepting();

        info.serverName = server->serverAddress().toString() + QLatin1Char(':') + QString::number(port);
        info.localSocket = false;
        info.socketDescriptor = server->socketDescriptor();
    }

    if (ret && info.socketDescriptor) {
        std::cout << "WSGI socket " << QByteArray::number(static_cast<int>(sockets.size())).constData()
                  << " bound to " << (info.localSocket ? "LOCAL" : "TCP") << " address " << info.serverName.toLatin1().constData()
                  << " fd " << QByteArray::number(info.socketDescriptor).constData()
                  << std::endl;
        sockets.push_back(info);
    } else {
        std::cout << "Failed to listen on LOCAL: " << line.toLatin1().constData() << std::endl;
        exit(1);
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
        d->threads = QThread::idealThreadCount();
    } else {
        d->threads = threads.toInt();
    }
}

QString WSGI::threads() const
{
    Q_D(const WSGI);
    return QString::number(d->threads);
}

void WSGI::setProcess(const QString &process)
{
    Q_D(WSGI);
    if (process.compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0) {
        d->process = QThread::idealThreadCount();
    } else {
        d->process = process.toInt();
    }
}

QString WSGI::process() const
{
    Q_D(const WSGI);
    return QString::number(d->process);
}

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

void WSGI::setHttpSocket(const QString &httpSocket)
{
    Q_D(WSGI);
    d->httpSockets.append(httpSocket.split(QLatin1Char(' '), QString::SkipEmptyParts));
}

QString WSGI::httpSocket() const
{
    Q_D(const WSGI);
    return d->httpSockets.join(QLatin1Char(' '));
}

void WSGI::setFastcgiSocket(const QString &fastcgiSocket)
{
    Q_D(WSGI);
    d->fastcgiSockets.append(fastcgiSocket.split(QLatin1Char(' '), QString::SkipEmptyParts));
}

QString WSGI::fastcgiSocket() const
{
    Q_D(const WSGI);
    return d->fastcgiSockets.join(QLatin1Char(' '));
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

void WSGI::setChdir2(const QString &chdir2)
{
    Q_D(WSGI);
    d->chdir = chdir2;
}

QString WSGI::chdir2() const
{
    Q_D(const WSGI);
    return d->chdir2;
}

void WSGI::setIni(const QString &ini)
{
    Q_D(WSGI);
    d->ini = ini;
}

QString WSGI::ini() const
{
    Q_D(const WSGI);
    return d->ini;
}

void WSGI::setStaticMap(const QString &staticMap)
{
    Q_D(WSGI);
    d->staticMaps.append(staticMap.split(QLatin1Char(';'), QString::SkipEmptyParts));
}

QString WSGI::staticMap() const
{
    Q_D(const WSGI);
    return d->staticMaps.join(QLatin1Char(';'));
}

void WSGI::setStaticMap2(const QString &staticMap)
{
    Q_D(WSGI);
    d->staticMaps2.append(staticMap.split(QLatin1Char(';'), QString::SkipEmptyParts));
}

QString WSGI::staticMap2() const
{
    Q_D(const WSGI);
    return d->staticMaps2.join(QLatin1Char(';'));
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
        setMaster(true); // master is required to restart app
    }
}

bool WSGI::autoReload() const
{
    Q_D(const WSGI);
    return d->autoReload;
}

void WSGI::setTouchReload(const QString &file)
{
    Q_D(WSGI);
    if (!file.isEmpty()) {
        d->touchReload.append(file.split(QLatin1Char(';'), QString::SkipEmptyParts));
        setMaster(true); // master is required to restart app
    }
}

QString WSGI::touchReload() const
{
    Q_D(const WSGI);
    return d->touchReload.join(QLatin1Char(';'));
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

void WSGIPrivate::proc()
{
    if (!masterChildProcess) {
        masterChildProcess = new QProcess(this);
        QObject::connect(masterChildProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, &WSGIPrivate::childFinished);

        if (autoReload) {
            touchReload.append(application);
        }

        if (!touchReload.isEmpty()) {
            auto watcher = new QFileSystemWatcher(touchReload, this);
            QObject::connect(watcher, &QFileSystemWatcher::fileChanged, this, &WSGIPrivate::restart);
        }
    }

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("CUTELYST_WSGI_IGNORE_MASTER"), QStringLiteral("1"));
    masterChildProcess->setProcessEnvironment(env);

    masterChildProcess->setProcessChannelMode(QProcess::ForwardedChannels);

    masterChildProcess->start(QCoreApplication::applicationFilePath(), QCoreApplication::arguments());

    delete materChildRestartTimer;
    materChildRestartTimer = nullptr;
}

void WSGIPrivate::parseCommandLine()
{
    Q_Q(WSGI);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Fast, developer-friendly WSGI server"));
    parser.addHelpOption();
    parser.addVersionOption();

    auto ini = QCommandLineOption(QStringLiteral("ini"),
                                  QCoreApplication::translate("main", "load config from ini file"),
                                  QCoreApplication::translate("main", "file"));
    parser.addOption(ini);

    auto chdir = QCommandLineOption(QStringLiteral("chdir"),
                                    QCoreApplication::translate("main", "chdir to specified directory before apps loading"),
                                    QCoreApplication::translate("main", "directory"));
    parser.addOption(chdir);

    auto chdir2 = QCommandLineOption(QStringLiteral("chdir2"),
                                     QCoreApplication::translate("main", "chdir to specified directory afterapps loading"),
                                     QCoreApplication::translate("main", "directory"));
    parser.addOption(chdir2);

    auto application = QCommandLineOption({ QStringLiteral("application"), QStringLiteral("a") },
                                          QCoreApplication::translate("main", "Application to load"),
                                          QCoreApplication::translate("main", "file"));
    parser.addOption(application);

    auto threads = QCommandLineOption({ QStringLiteral("threads"), QStringLiteral("t") },
                                      QCoreApplication::translate("main", "Number of thread to use"),
                                      QCoreApplication::translate("main", "threads"));
    parser.addOption(threads);

#ifdef Q_OS_UNIX
    auto process = QCommandLineOption({ QStringLiteral("process"), QStringLiteral("p") },
                                      QCoreApplication::translate("main", "spawn the specified number of processes"),
                                      QCoreApplication::translate("main", "processes"));
    parser.addOption(process);
#endif // Q_OS_UNIX

    auto master = QCommandLineOption({ QStringLiteral("master"), QStringLiteral("M") },
                                      QCoreApplication::translate("main", "Enable master process"));
    parser.addOption(master);

    auto bufferSize = QCommandLineOption({ QStringLiteral("buffer-size"), QStringLiteral("b") },
                                         QCoreApplication::translate("main", "set internal buffer size"),
                                         QCoreApplication::translate("main", "bytes"));
    parser.addOption(bufferSize);

    auto postBuffering = QCommandLineOption(QStringLiteral("post-buffering"),
                                            QCoreApplication::translate("main", "set size after which will buffer to disk instead of memory"),
                                            QCoreApplication::translate("main", "bytes"));
    parser.addOption(postBuffering);

    auto postBufferingBufsize = QCommandLineOption(QStringLiteral("post-buffering-bufsize"),
                                            QCoreApplication::translate("main", "set buffer size for read() in post buffering mode"),
                                            QCoreApplication::translate("main", "bytes"));
    parser.addOption(postBufferingBufsize);

    auto httpSocket = QCommandLineOption({ QStringLiteral("http-socket"), QStringLiteral("h1") },
                                         QCoreApplication::translate("main", "bind to the specified TCP socket using HTTP protocol"),
                                         QCoreApplication::translate("main", "address"));
    parser.addOption(httpSocket);

    auto fastcgiSocket = QCommandLineOption(QStringLiteral("fastcgi-socket"),
                                            QCoreApplication::translate("main", "bind to the specified UNIX/TCP socket using FastCGI protocol"),
                                            QCoreApplication::translate("main", "address"));
    parser.addOption(fastcgiSocket);

    auto socketAccess = QCommandLineOption(QStringLiteral("socket-access"),
                                           QCoreApplication::translate("main", "set the LOCAL socket access, such as 'ugo' standing for User, Group, Other access"),
                                           QCoreApplication::translate("main", "options"));
    parser.addOption(socketAccess);

    auto staticMap = QCommandLineOption(QStringLiteral("static-map"),
                                        QCoreApplication::translate("main", "map mountpoint to static directory (or file)"),
                                        QCoreApplication::translate("main", "mountpoint=path"));
    parser.addOption(staticMap);

    auto staticMap2 = QCommandLineOption(QStringLiteral("static-map2"),
                                         QCoreApplication::translate("main", "like static-map but completely appending the requested resource to the docroot"),
                                         QCoreApplication::translate("main", "mountpoint=path"));
    parser.addOption(staticMap2);

    auto autoReload = QCommandLineOption({ QStringLiteral("auto-restart"), QStringLiteral("r") },
                                      QCoreApplication::translate("main", "auto restarts when the application file changes"));
    parser.addOption(autoReload);

    auto touchReload = QCommandLineOption(QStringLiteral("touch-reload"),
                                        QCoreApplication::translate("main", "reload application if the specified file is modified/touched"),
                                        QCoreApplication::translate("main", "file"));
    parser.addOption(touchReload);

    auto tcpNoDelay = QCommandLineOption(QStringLiteral("tcp-nodelay"),
                                         QCoreApplication::translate("main", "enable TCP NODELAY on each request"));
    parser.addOption(tcpNoDelay);

    auto soKeepAlive = QCommandLineOption(QStringLiteral("so-keepalive"),
                                          QCoreApplication::translate("main", "enable TCP KEEPALIVEs"));
    parser.addOption(soKeepAlive);

    auto socketSndbuf = QCommandLineOption(QStringLiteral("socket-sndbuf"),
                                           QCoreApplication::translate("main", "set SO_SNDBUF"),
                                           QCoreApplication::translate("main", "bytes"));
    parser.addOption(socketSndbuf);

    auto socketRcvbuf = QCommandLineOption(QStringLiteral("socket-rcvbuf"),
                                           QCoreApplication::translate("main", "set SO_RCVBUF"),
                                           QCoreApplication::translate("main", "bytes"));
    parser.addOption(socketRcvbuf);

    // Process the actual command line arguments given by the user
    parser.process(*qApp);

    if (parser.isSet(ini)) {
        q->setIni(parser.value(ini));
    }

    if (parser.isSet(chdir)) {
        q->setChdir(parser.value(chdir));
    }

    if (parser.isSet(chdir2)) {
        q->setChdir(parser.value(chdir2));
    }

    if (parser.isSet(threads)) {
        q->setThreads(parser.value(threads));
    }

    if (parser.isSet(socketAccess)) {
        q->setSocketAccess(parser.value(socketAccess));
    }

#ifdef Q_OS_UNIX
    if (parser.isSet(process)) {
        q->setProcess(parser.value(process));
    }
#endif // Q_OS_UNIX

    if (parser.isSet(bufferSize)) {
        bool ok;
        auto size = parser.value(bufferSize).toLongLong(&ok);
        q->setBufferSize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBuffering)) {
        bool ok;
        auto size = parser.value(postBuffering).toLongLong(&ok);
        q->setPostBuffering(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(postBufferingBufsize)) {
        bool ok;
        auto size = parser.value(postBufferingBufsize).toLongLong(&ok);
        q->setPostBufferingBufsize(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(application)) {
        q->setApplication(parser.value(application));
    }

    bool masterSet = parser.isSet(master);
    q->setMaster(masterSet);

    q->setAutoReload(parser.isSet(autoReload));

    q->setTcpNodelay(parser.isSet(tcpNoDelay));

    q->setSoKeepalive(parser.isSet(soKeepAlive));

    if (parser.isSet(socketSndbuf)) {
        bool ok;
        auto size = parser.value(socketSndbuf).toInt(&ok);
        q->setSocketSndbuf(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (parser.isSet(socketRcvbuf)) {
        bool ok;
        auto size = parser.value(socketRcvbuf).toInt(&ok);
        q->setSocketRcvbuf(size);
        if (!ok || size < 1) {
            parser.showHelp(1);
        }
    }

    if (!masterSet && parser.isSet(httpSocket)) {
        const auto socks = parser.values(httpSocket);
        for (const QString &http : socks) {
            q->setHttpSocket(http);
        }
    }

    if (!masterSet && parser.isSet(fastcgiSocket)) {
        const auto socks = parser.values(fastcgiSocket);
        for (const QString &sock : socks) {
            q->setFastcgiSocket(sock);
        }
    }

    if (parser.isSet(staticMap)) {
        const auto maps = parser.values(staticMap);
        for (const QString &map : maps) {
            q->setStaticMap(map);
        }
    }

    if (parser.isSet(staticMap2)) {
        const auto maps = parser.values(staticMap2);
        for (const QString &map : maps) {
            q->setStaticMap2(map);
        }
    }

    if (parser.isSet(touchReload)) {
        const auto maps = parser.values(touchReload);
        for (const QString &map : maps) {
            q->setTouchReload(map);
        }
    }
}

int WSGIPrivate::setupApplication(Cutelyst::Application *app)
{
    Q_Q(WSGI);

    if (!chdir.isEmpty()) {
        std::cout << "Changing directory to: " << chdir.toLatin1().constData() << std::endl;;
        if (!QDir::setCurrent(chdir)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << chdir;
            return 1;
        }
    }

    if (!httpSockets.isEmpty()) {
        protoHTTP = new ProtocolHttp(q);
        const auto sockets = httpSockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoHTTP);
        }
    }

    if (!fastcgiSockets.isEmpty()) {
        protoFCGI = new ProtocolFastCGI(q);
        const auto sockets = fastcgiSockets;
        for (const auto &socket : sockets) {
            listenTcp(socket, protoFCGI);
        }
    }

    if (!sockets.size()) {
        std::cout << "Please specify a socket to listen to" << std::endl;
        return 1;
    }

    Cutelyst::Application *localApp = app;
    if (!localApp) {
        std::cout << "Loading application: " << application.toLatin1().constData() << std::endl;;
        QPluginLoader loader(application);
        if (!loader.load()) {
            qCCritical(CUTELYST_WSGI) << "Could not load application:" << loader.errorString();
            return 1;
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

    engine = createEngine(localApp, 0);
    if (!engine->init()) {
        qCCritical(CUTELYST_WSGI) << "Failed to init application.";
        exit(1);
    }

    std::cout << "Threads:" << threads << std::endl;
    if (threads) {
        enginesInitted = threads;
        for (int i = 1; i < threads; ++i) {
            createEngine(localApp, i);
        }
    }

    if (!chdir2.isEmpty()) {
        std::cout << "Changing directory2 to" << chdir2.toLatin1().constData()  << std::endl;;
        if (!QDir::setCurrent(chdir2)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << chdir2;
            return 1;
        }
    }

// TODO create a listening timer

    return 0;
}

void WSGIPrivate::childFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(CUTELYST_WSGI) << "Master child finished" << exitCode << exitStatus;
    if (materChildRestartTimer || exitStatus == QProcess::CrashExit) {
        proc();
    } else {
        qApp->exit(exitCode);
    }
}

void WSGIPrivate::restart(const QString &path)
{
    qCDebug(CUTELYST_WSGI) << "File changed restarting" << path;
    if (!materChildRestartTimer) {
        materChildRestartTimer = new QTimer(this);
        materChildRestartTimer->setInterval(1 * 1000);
        materChildRestartTimer->setSingleShot(false);

        QObject::connect(materChildRestartTimer, &QTimer::timeout, this, &WSGIPrivate::restartTerminate);
    }
    materChildRestartTimer->start();
}

void WSGIPrivate::restartTerminate()
{
    if (++autoReloadCount > 5) {
        masterChildProcess->kill();
    } else {
        masterChildProcess->terminate();
    }
}

void WSGIPrivate::engineInitted()
{
    // All engines are initted
    if (--enginesInitted == 0) {
#ifdef Q_OS_UNIX
        if (process) {
            auto uFork = new UnixFork(this);
            connect(uFork, &UnixFork::forked, this, &WSGIPrivate::forked);
            uFork->createProcess(process);
        } else {
            Q_EMIT forked();
        }
#else
        Q_EMIT forked();
#endif //Q_OS_UNIX
    }
}

CWsgiEngine *WSGIPrivate::createEngine(Application *app, int core)
{
    Q_Q(WSGI);

    auto engine = new CWsgiEngine(app, core, QVariantMap(), q);
    connect(engine, &CWsgiEngine::initted, this, &WSGIPrivate::engineInitted, Qt::QueuedConnection);
    connect(this, &WSGIPrivate::forked, engine, &CWsgiEngine::postFork, Qt::QueuedConnection);
    engine->setTcpSockets(sockets);
    engines.push_back(engine);

    if (threads && core) {
        auto thread = new QThread(this);
#ifdef Q_OS_LINUX
        if (qEnvironmentVariableIsSet("CUTELYST_EVENT_LOOP_EPOLL")) {
            thread->setEventDispatcher(new EventDispatcherEPoll);
        }
#endif
        engine->moveToThread(thread);
        connect(thread, &QThread::started, engine, &CWsgiEngine::listen, Qt::DirectConnection);
        thread->start();
    } else {
        engine->listen();
    }
    return engine;
}

bool WSGIPrivate::loadConfig()
{
    QSettings settings(ini, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    qputenv("CUTELYST_CONFIG", ini.toUtf8());
    if (!qEnvironmentVariableIsSet("QT_LOGGING_CONF")) {
        qputenv("QT_LOGGING_CONF", ini.toUtf8());
    }

    loadConfigGroup(QStringLiteral("uwsgi"), settings);
    loadConfigGroup(QStringLiteral("wsgi"), settings);

    return true;
}

void WSGIPrivate::loadConfigGroup(const QString &group, QSettings &settings)
{
    Q_Q(WSGI);

    settings.beginGroup(group);
    const auto keys = settings.allKeys();
    for (const QString &key : keys) {
        QString prop = key;
        prop.replace(QLatin1Char('-'), QLatin1Char('_'));
        const QVariant value = settings.value(key);
        if (value.type() == QVariant::StringList) {
            const auto values = value.toStringList();
            for (const auto &str : values) {
                q->setProperty(prop.toLatin1().constData(), str);
            }
        } else {
            q->setProperty(prop.toLatin1().constData(), value);
        }
    }
    settings.endGroup();
}

#include "moc_wsgi.cpp"
