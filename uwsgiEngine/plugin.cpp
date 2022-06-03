/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "engineuwsgi.h"

#include "uwsgiconnection.h"

#include <uwsgi.h>

#include <Cutelyst/application.h>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QPluginLoader>
#include <QFileInfo>
#include <QDir>

#ifdef Q_OS_LINUX
#include "../EventLoopEPoll/eventdispatcher_epoll.h"
#endif

#define CUTELYST_MODIFIER1 0

using namespace Cutelyst;

struct uwsgi_cutelyst {
    char *app;
} options;

static QVector<uWSGI *> *coreEngines = nullptr;
static QVariantMap *config;

void cuteOutput(QtMsgType, const QMessageLogContext &, const QString &);
void uwsgi_cutelyst_loop(void);

/**
 * This function is called as soon as
 * the plugin is loaded
 */
void uwsgi_cutelyst_on_load()
{
    uwsgi_register_loop( (char *) "CutelystQtLoop", uwsgi_cutelyst_loop);
    config = new QVariantMap;

    // Get the uwsgi options
    QVariantMap opts;
    for (int i = 0; i < uwsgi.exported_opts_cnt; i++) {
        const QString key = QString::fromLatin1(uwsgi.exported_opts[i]->key);
        if (uwsgi.exported_opts[i]->value == NULL) {
            opts.insertMulti(key, QVariant());
        } else {
            opts.insertMulti(key, QString::fromLatin1(uwsgi.exported_opts[i]->value));
        }
    }

    // if the path is relative build a path
    // relative to the current working directory
    QDir cwd(QString::fromLatin1(uwsgi.cwd));

    // Set the configuration env
    auto it = opts.constFind(QLatin1String("ini"));
    if (it != opts.constEnd()) {
        const QString ini = cwd.absoluteFilePath(it.value().toString());
        QVariantMap iniConfig = Engine::loadIniConfig(ini);
        config->unite(iniConfig);
        if (!qEnvironmentVariableIsSet("QT_LOGGING_CONF")) {
            qputenv("QT_LOGGING_CONF", ini.toUtf8());
        }
    }

#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsSet("CUTELYST_EVENT_LOOP_EPOLL")) {
        uwsgi_log("Installing EPoll event loop");
        QCoreApplication::setEventDispatcher(new EventDispatcherEPoll);
    }
#endif

    auto app = new QCoreApplication(uwsgi.argc, uwsgi.argv);
    app->setProperty("UWSGI_OPTS", opts);

    if (qEnvironmentVariableIsSet("CUTELYST_UWSGI_LOG")) {
        qInstallMessageHandler(cuteOutput);
    }

    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qSetMessagePattern(QLatin1String("%{category}[%{type}] %{message}"));
    }
}

int uwsgi_cutelyst_init()
{
    uwsgi_log("Initializing Cutelyst plugin\n");

    // if the path is relative build a path
    // relative to the current working directory
    QDir cwd(QString::fromLocal8Bit(uwsgi.cwd));

    QString path(QString::fromLocal8Bit(options.app));
    if (path.isEmpty()) {
        uwsgi_log("Cutelyst application name or path was not set\n");
        exit(1);
    }

    path = cwd.absoluteFilePath(path);

    uwsgi.loop = (char *) "CutelystQtLoop";

    coreEngines = new QVector<uWSGI *>();

    return 0;
}

void uwsgi_cutelyst_post_fork()
{
    const auto engines = *coreEngines;
    for (uWSGI *engine : engines) {
        engine->setWorkerId(uwsgi.mywid - 1);
        if (engine->thread() != qApp->thread()) {
            engine->thread()->start();
        } else {
            Q_EMIT engine->postFork();
        }
    }
}

int uwsgi_cutelyst_request(struct wsgi_request *wsgi_req)
{
    // empty request ?
    if (!wsgi_req->uh->pktsize) {
        qCDebug(CUTELYST_UWSGI) << "Empty request. skip.";
        return -1;
    }

    // get uwsgi variables
    if (uwsgi_parse_vars(wsgi_req)) {
        qCDebug(CUTELYST_UWSGI) << "Invalid request. skip.";
        return -1;
    }

    uwsgiConnection req(wsgi_req);
    coreEngines->at(wsgi_req->async_id)->processRequest(&req);

    return UWSGI_OK;
}

/**
 * This function is called when the child process is exiting
 */
void uwsgi_cutelyst_atexit()
{
    qCDebug(CUTELYST_UWSGI) << "Child process finishing:" << QCoreApplication::applicationPid();

    const auto engines = *coreEngines;
    for (uWSGI *engine : engines) {
        engine->stop();
        delete engine;
    }

    delete coreEngines;
    delete config;

    qCDebug(CUTELYST_UWSGI) << "Child process finished:" << QCoreApplication::applicationPid();
}

void uwsgi_cutelyst_init_apps()
{
    const QString applicationName = QCoreApplication::applicationName();

    qCDebug(CUTELYST_UWSGI) << "Cutelyst loading application:" << options.app;

    // if the path is relative build a path
    // relative to the current working directory
    QDir cwd(QString::fromLocal8Bit(uwsgi.cwd));
    QString path = cwd.absoluteFilePath(QString::fromLocal8Bit(options.app));

    auto loader = new QPluginLoader(path);
    if (!loader->load()) {
        qCCritical(CUTELYST_UWSGI) << "Could not load application:" << loader->errorString();
        exit(1);
    }

    QObject *instance = loader->instance();
    if (!instance) {
        qCCritical(CUTELYST_UWSGI) << "Could not get a QObject instance: %s\n" << loader->errorString();
        exit(1);
    }

    auto app = qobject_cast<Application *>(instance);
    if (!app) {
        qCCritical(CUTELYST_UWSGI) << "Could not cast Cutelyst::Application from instance: %s\n" << loader->errorString();
        exit(1);
    }

    // Sets the application name with the name from our library
    if (QCoreApplication::applicationName() == applicationName) {
        QCoreApplication::setApplicationName(QString::fromLatin1(app->metaObject()->className()));
    }
    qCDebug(CUTELYST_UWSGI) << "Loaded application:" << QCoreApplication::applicationName();

    QVariantMap opts = qApp->property("UWSGI_OPTS").toMap();

    auto mainEngine = new uWSGI(app, 0, opts);
    mainEngine->setConfig(*config);
    if (!mainEngine->init()) {
        qCCritical(CUTELYST_UWSGI) << "Failed to init application.";
        exit(1);
    }

    uWSGI *engine = mainEngine;
    for (int i = 0; i < uwsgi.cores; ++i) {
        // Create the wsgi_request structure
        struct wsgi_request *wsgi_req = new wsgi_request;
        memset(wsgi_req, 0, sizeof(struct wsgi_request));
        wsgi_req->async_id = i;

        // Create the desired threads
        // i > 0 the main thread counts as one thread
        if (uwsgi.threads > 1) {
            auto thread = new QThread(qApp);
#ifdef Q_OS_LINUX
            if (qEnvironmentVariableIsSet("CUTELYST_EVENT_LOOP_EPOLL")) {
                thread->setEventDispatcher(new EventDispatcherEPoll);
            }
#endif

            // reuse the main engine
            if (i != 0) {
                // The engine can't have a parent otherwise
                // we can't move it
                engine = new uWSGI(app, i, opts);
                engine->setConfig(*config);
            }

            // the request must be added before moving threads
            engine->addUnusedRequest(wsgi_req);

            // Move to the new thread
            engine->setThread(thread);
        } else {
            engine->addUnusedRequest(wsgi_req);
        }

        if (!coreEngines->contains(engine)) {
            coreEngines->append(engine);
        }
    }

    // register a new app under a specific "mountpoint"
    uwsgi_add_app(1, CUTELYST_MODIFIER1, (char *) "", 0, NULL, NULL);

    delete loader;

    if (uwsgi.lazy || uwsgi.lazy_apps) {
        // Make sure we start listening on lazy mode
        uwsgi_cutelyst_post_fork();
    }
}

void uwsgi_cutelyst_watch_signal(int signalFD)
{
    auto socketNotifier = new QSocketNotifier(signalFD, QSocketNotifier::Read);
    QObject::connect(socketNotifier, &QSocketNotifier::activated,
                     [=](int fd) {
        socketNotifier->setEnabled(false);
        uwsgi_receive_signal(fd, (char *) "worker", uwsgi.mywid);
        socketNotifier->setEnabled(true);
    });
}

void uwsgi_cutelyst_loop()
{
    // ensure SIGPIPE is ignored
    signal(SIGPIPE, SIG_IGN);

    // FIX for some reason this is not being set by UWSGI
    uwsgi.wait_read_hook = uwsgi_simple_wait_read_hook;

    // monitor signals
    if (uwsgi.signal_socket > -1) {
        uwsgi_cutelyst_watch_signal(uwsgi.signal_socket);
        uwsgi_cutelyst_watch_signal(uwsgi.my_signal_socket);
    }

    // start the qt event loop
    qApp->exec();
}

void cuteOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        uwsgi_log("%s[debug] %s\n", context.category, localMsg.constData());
        break;
    case QtWarningMsg:
        uwsgi_log("%s[warn]  %s\n", context.category, localMsg.constData());
        break;
    case QtCriticalMsg:
        uwsgi_log("%s[crit]  %s\n", context.category, localMsg.constData());
        break;
    case QtFatalMsg:
        uwsgi_log("%s[fatal] %s\n", context.category, localMsg.constData());
        abort();
    case QtInfoMsg:
        uwsgi_log("%s[info]  %s\n", context.category, localMsg.constData());
        break;
    }
}

struct uwsgi_option uwsgi_cutelyst_options[] = {

{const_cast<char *>("cutelyst-app"), required_argument, 0, const_cast<char *>("loads the Cutelyst Application"), uwsgi_opt_set_str, &options.app, 0},
{0, 0, 0, 0, 0, 0, 0},

};

struct uwsgi_plugin CUTELYST_LIBRARY cutelyst2_plugin = {
    "cutelyst", // name
    0, // alias
    0, // modifier1
    0, // data
    uwsgi_cutelyst_on_load, // on_load
    uwsgi_cutelyst_init, // init
    0, // post_init
    uwsgi_cutelyst_post_fork, // post_fork
    uwsgi_cutelyst_options, // options
    0, // enable threads
    0, // init thread
    uwsgi_cutelyst_request, // request
    0, // after request
    0, // pre init apps
    uwsgi_cutelyst_init_apps, // init apps
    0, // post init apps
    0, // (*fixup) (void);
    0, //void (*master_fixup) (int);
    0, // master_cycle) (void);
    0, //int (*mount_app) (char *, char *);
    0, //int (*manage_udp) (char *, int, char *, int);
    0, //void (*suspend) (struct wsgi_request *);
    0, //void (*resume) (struct wsgi_request *);

    0, //void (*harakiri) (int);

    0, //void (*hijack_worker) (void);
    0, //void (*spooler_init) (void);
    uwsgi_cutelyst_atexit, // atexit
};
