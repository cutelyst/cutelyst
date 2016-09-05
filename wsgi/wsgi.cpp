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
#include "wsgi.h"

#include "protocol.h"
#include "protocolhttp.h"
#include "cwsgiengine.h"
#include "tcpserver.h"
#include "socket.h"

#ifdef Q_OS_UNIX
#include "unixfork.h"
#endif

#include <Cutelyst/Application>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPluginLoader>
#include <QThread>
#include <QLoggingCategory>
#include <QSettings>
#include <QDir>

#include <iostream>

Q_LOGGING_CATEGORY(CUTELYST_WSGI, "cutelyst.wsgi")

using namespace CWSGI;

WSGI::WSGI(QObject *parent) : QObject(parent)
{
    std::cout << "WSGI starting" << std::endl;
}


int WSGI::load(const QCoreApplication &app)
{
    int ret = parseCommandLine(app);
    if (ret) {
        return ret;
    }

    if (m_master) {
        proc();
        return 0;
    }

    return setupApplication();
}

bool WSGI::listenTcp(const QString &line)
{
    QStringList parts = line.split(QLatin1Char(':'));
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
    bool ret = server->listen(address, port);
    server->pauseAccepting();
    m_sockets.append(server);

    if (ret) {
        std::cout << "Listening on: "
                  << server->serverAddress().toString().toLatin1().constData() << ':' << server->serverPort() << std::endl;
    } else {
        std::cout << "Failed to listen on: " << line.toLatin1().constData() << std::endl;
        exit(1);
    }

    return ret;
}

bool WSGI::listenSocket(const QString &address)
{
    auto server = new QLocalServer(this);
    //    connect(server, &QLocalServer::newConnection, this, &WSGI::newconnectionLocalSocket);
    return server->listen(address);
}

void WSGI::setApplication(const QString &application)
{
    m_application = application;
}

QString WSGI::application() const
{
    return m_application;
}

void WSGI::setThreads(int threads)
{
    m_threads = threads;
}

int WSGI::threads() const
{
    return m_threads;
}

void WSGI::setProcess(int process)
{
    m_process = process;
}

int WSGI::process() const
{
    return m_process;
}

void WSGI::setChdir(const QString &chdir)
{
    m_chdir = chdir;
}

QString WSGI::chdir() const
{
    return m_chdir;
}

void WSGI::setHttpSocket(const QString &httpSocket)
{
    listenTcp(httpSocket);
}

QString WSGI::httpSocket() const
{
    return QString();
}

void WSGI::setChdir2(const QString &chdir2)
{
    m_chdir = chdir2;
}

QString WSGI::chdir2() const
{
    return m_chdir2;
}

void WSGI::setIni(const QString &ini)
{
    m_ini = ini;
}

QString WSGI::ini() const
{
    return m_ini;
}

void WSGI::setMaster(bool enable)
{
    m_master = enable;
}

bool WSGI::master() const
{
    return m_master;
}

void WSGI::setBufferSize(qint64 size)
{
    if (size < 4096) {
        qWarning() << "Buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    m_bufferSize = size;
}

int WSGI::bufferSize() const
{
    return m_bufferSize;
}

void WSGI::setPostBuffering(qint64 size)
{
    m_postBuffering = size;
}

qint64 WSGI::postBuffering() const
{
    return m_postBuffering;
}

void WSGI::setPostBufferingBufsize(qint64 size)
{
    if (size < 4096) {
        qWarning() << "Post buffer size must be at least 4096 bytes, ignoring";
        return;
    }
    m_postBufferingBufsize = size;
}

qint64 WSGI::postBufferingBufsize() const
{
    return m_postBufferingBufsize;
}

void WSGI::proc()
{
    static QProcess *process = nullptr;
    if (!process) {
        process = new QProcess(this);
        connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, &WSGI::childFinished);
    }

    const QString app = QCoreApplication::applicationFilePath();
    QStringList args = QCoreApplication::arguments();
    args.takeFirst();
    args.removeOne(QStringLiteral("-M"));
    args.removeOne(QStringLiteral("--master"));

    process->setProcessChannelMode(QProcess::ForwardedChannels);

    process->start(app, args);
}

int WSGI::parseCommandLine(const QCoreApplication &app)
{
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

    QCommandLineOption restart = QCommandLineOption({ QStringLiteral("restart"), QStringLiteral("r") },
                                                    QStringLiteral("Restarts when the application file changes"));
    parser.addOption(restart);

    // Process the actual command line arguments given by the user
    parser.process(app);

    if (parser.isSet(ini)) {
        setIni(parser.value(ini));
    }

    if (parser.isSet(chdir)) {
        setChdir(parser.value(chdir));
    }

    if (parser.isSet(chdir2)) {
        setChdir(parser.value(chdir2));
    }

    if (parser.isSet(threads)) {
        setThreads(parser.value(threads).toInt());
    }

#ifdef Q_OS_UNIX
    if (parser.isSet(process)) {
        setProcess(parser.value(process).toInt());
    }
#endif // Q_OS_UNIX

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

    bool masterSet = parser.isSet(master);
    setMaster(masterSet);

    if (!masterSet && parser.isSet(httpSocket)) {
        const auto socks = parser.values(httpSocket);
        for (const QString &http : socks) {
            setHttpSocket(http);
        }
    }

    if (this->application().isEmpty()) {
        std::cout << "Application is not defined" << std::endl;
        parser.showHelp(2);
    }

    return 0;
}

int WSGI::setupApplication()
{
    if (!m_chdir.isEmpty()) {
        std::cout << "Changing directory to: " << m_chdir.toLatin1().constData() << std::endl;;
        if (!QDir().cd(m_chdir)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << m_chdir;
            return 1;
        }
    }

    if (!m_ini.isEmpty()) {
        std::cout << "Loading configuration: " << m_ini.toLatin1().constData() << std::endl;;
        if (!loadConfig()) {
            qCCritical(CUTELYST_WSGI) << "Failed to load config " << m_ini;
            return 1;
        }
    }

    if (!m_sockets.size()) {
        std::cout << "Please specify a socket to listen to" << std::endl;
        return false;
    }

    std::cout << "Loading application: " << m_application.toLatin1().constData() << std::endl;;
    QPluginLoader loader(m_application);
    if (!loader.load()) {
        qCritical() << "Could not load application:" << loader.errorString();
        return 1;
    }

    QObject *instance = loader.instance();
    if (!instance) {
        qCritical() << "Could not get a QObject instance: %s\n" << loader.errorString();
        exit(1);
    }

    Cutelyst::Application *app = qobject_cast<Cutelyst::Application *>(instance);
    if (!app) {
        qCritical() << "Could not cast Cutelyst::Application from instance: %s\n" << loader.errorString();
        exit(1);
    }

    // Sets the application name with the name from our library
    //    if (QCoreApplication::applicationName() == applicationName) {
    //        QCoreApplication::setApplicationName(QString::fromLatin1(app->metaObject()->className()));
    //    }
    qDebug() << "Loaded application: " << QCoreApplication::applicationName();

    m_engine = createEngine(app, 0);
    if (!m_engine->init()) {
        qCritical() << "Failed to init application.";
        exit(1);
    }

    std::cout << "Threads:" << m_threads << std::endl;
    if (m_threads) {
        m_enginesInitted = m_threads;
        for (int i = 1; i < m_threads; ++i) {
            createEngine(app, i);
        }
    }

    if (!m_chdir2.isEmpty()) {
        std::cout << "Changing directory2 to" << m_chdir2.toLatin1().constData()  << std::endl;;
        if (!QDir().cd(m_chdir2)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << m_chdir2;
            return 1;
        }
    }

// TODO create a listening timer

    return 0;
}

void WSGI::childFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        proc();
    } else {
        qApp->exit(exitCode);
    }
}

void WSGI::engineInitted()
{
    // All engines are initted
    if (--m_enginesInitted == 0) {
#ifdef Q_OS_UNIX
        if (m_process) {
            auto uFork = new UnixFork(this);
            connect(uFork, &UnixFork::forked, this, &WSGI::forked);
            uFork->createProcess(m_process);
        } else {
            Q_EMIT forked();
        }
#else
        Q_EMIT forked();
#endif //Q_OS_UNIX
    }
}

CWsgiEngine *WSGI::createEngine(Application *app, int core)
{
    auto engine = new CWsgiEngine(app, core, QVariantMap(), this);
    connect(engine, &CWsgiEngine::initted, this, &WSGI::engineInitted, Qt::QueuedConnection);
    connect(this, &WSGI::forked, engine, &CWsgiEngine::postFork, Qt::QueuedConnection);
    engine->setTcpSockets(m_sockets);
    m_engines.push_back(engine);

    if (m_threads && core) {
        auto thread = new QThread(this);
        engine->moveToThread(thread);
        connect(thread, &QThread::started, engine, &CWsgiEngine::listen, Qt::DirectConnection);
        thread->start();
    } else {
        engine->listen();
    }
    return engine;
}

bool WSGI::loadConfig()
{
    QSettings settings(m_ini, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    qputenv("CUTELYST_CONFIG", m_ini.toUtf8());
    if (!qEnvironmentVariableIsSet("QT_LOGGING_CONF")) {
        qputenv("QT_LOGGING_CONF", m_ini.toUtf8());
    }

    settings.beginGroup(QStringLiteral("wsgi"));
    const auto keys = settings.allKeys();
    for (const QString &key : keys) {
        QString prop = key;
        prop.replace(QLatin1Char('-'), QLatin1Char('_'));
        setProperty(prop.toLatin1().constData(), settings.value(key));
    }

    return true;
}

#include "moc_wsgi.cpp"
