#include "wsgi.h"

#include "protocol.h"
#include "protocolhttp.h"
#include "cuteengine.h"
#include "tcpserver.h"
#include "socket.h"

#include <Cutelyst/Application>

#include <QCoreApplication>
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

WSGI::WSGI(QObject *parent) : QObject(parent)
{
    std::cout << "WSGI starting" << std::endl;
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
        std::cout << "Failed to listen on" << line.toLatin1().constData() << std::endl;
    }

    return ret;
}

bool WSGI::listenSocket(const QString &address)
{
    auto server = new QLocalServer(this);
    //    connect(server, &QLocalServer::newConnection, this, &WSGI::newconnectionLocalSocket);
    return server->listen(address);
}

bool WSGI::loadApplication()
{
    if (!m_chdir.isEmpty()) {
        std::cout << "Changing directory to: " << m_chdir.toLatin1().constData() << std::endl;;
        if (!QDir().cd(m_chdir)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << m_chdir;
            return false;
        }
    }

    if (!m_ini.isEmpty()) {
        std::cout << "Loading configuration: " << m_ini.toLatin1().constData() << std::endl;;
        if (!loadConfig()) {
            qCCritical(CUTELYST_WSGI) << "Failed to load config " << m_ini;
            return false;
        }
    }

    if (!m_sockets.size()) {
        std::cout << "Please specify a socket to listen to" << std::endl;
        return false;
    }

    std::cout << "Loading application: " << m_application.toLatin1().constData() << std::endl;;
    QPluginLoader *loader = new QPluginLoader(m_application);
    if (!loader->load()) {
        qCritical() << "Could not load application:" << loader->errorString();
        return false;
    }

    QObject *instance = loader->instance();
    if (!instance) {
        qCritical() << "Could not get a QObject instance: %s\n" << loader->errorString();
        exit(1);
    }

    Cutelyst::Application *app = qobject_cast<Cutelyst::Application *>(instance);
    if (!app) {
        qCritical() << "Could not cast Cutelyst::Application from instance: %s\n" << loader->errorString();
        exit(1);
    }

    // Sets the application name with the name from our library
    //    if (QCoreApplication::applicationName() == applicationName) {
    //        QCoreApplication::setApplicationName(QString::fromLatin1(app->metaObject()->className()));
    //    }
    qDebug() << "Loaded application: " << QCoreApplication::applicationName();

    m_engine = createEngine(app, 0);
    if (!m_engine->initApplication(app, false)) {
        qCritical() << "Failed to init application.";
        exit(1);
    }
    m_engine->forked();

    std::cout << "Threads:" << m_threads << std::endl;
    if (m_threads) {
        for (int i = 1; i < m_threads; ++i) {
            createEngine(app, i);
        }
    }

    if (!m_chdir2.isEmpty()) {
        std::cout << "Changing directory2 to" << m_chdir2.toLatin1().constData()  << std::endl;;
        if (!QDir().cd(m_chdir2)) {
            qCCritical(CUTELYST_WSGI) << "Failed to chdir to" << m_chdir2;
            return false;
        }
    }

    return true;
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

CuteEngine *WSGI::createEngine(Application *app, int core)
{
    auto engine = new CuteEngine(QVariantMap(), app);
    engine->m_workerCore = core;
    engine->setTcpSockets(m_sockets);
    m_engines.push_back(engine);

    if (m_threads) {
        auto t1 = new QThread(this);
        engine->moveToThread(t1);
        connect(t1, &QThread::started, engine, &CuteEngine::forked);
        t1->start();
    }
    return engine;
}

bool WSGI::loadConfig()
{
    QSettings settings(m_ini, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    settings.beginGroup(QStringLiteral("wsgi"));
    const auto keys = settings.allKeys();
    for (const QString &key : keys) {
        setProperty(key.toLatin1().constData(), settings.value(key));
    }

    return true;
}

#include "moc_wsgi.cpp"
