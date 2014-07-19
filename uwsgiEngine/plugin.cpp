/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "engineuwsgi.h"
#include "plugin.h"
#include "mainthreaduwsgi.h"

#include <Cutelyst/Application>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QPluginLoader>

using namespace Cutelyst;

static QList<EngineUwsgi *> coreEngines;
static QList<QThread *> coreThreads;

void cuteOutput(QtMsgType, const QMessageLogContext &, const QString &);
void uwsgi_cutelyst_loop(void);

/**
 * This function is called as soon as
 * the plugin is loaded
 */
extern "C" void uwsgi_cutelyst_on_load()
{
    uwsgi_register_loop( (char *) "CutelystQtLoop", uwsgi_cutelyst_loop);

    (void) new QCoreApplication(uwsgi.argc, uwsgi.argv);

    qInstallMessageHandler(cuteOutput);
}

extern "C" int uwsgi_cutelyst_init()
{
    qCDebug(CUTELYST_UWSGI) << "Initializing Cutelyst plugin";
    qCDebug(CUTELYST_UWSGI) << "-> async" << uwsgi.async << "-> threads" << uwsgi.threads;
    if (uwsgi.async < uwsgi.threads) {
        uwsgi_log("--async must be greater or equal to --threads value\n");
        exit(1);
    }

    uwsgi.loop = (char *) "CutelystQtLoop";

    return 0;
}

extern "C" void uwsgi_cutelyst_post_fork()
{
    Q_FOREACH (QThread *thread, coreThreads) {
        thread->start();
    }

    Q_FOREACH (EngineUwsgi *engine, coreEngines) {
        Q_EMIT engine->postFork();
    }
}

extern "C" int uwsgi_cutelyst_request(struct wsgi_request *wsgi_req)
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

    coreEngines.at(wsgi_req->async_id)->receiveRequest(wsgi_req);

    return UWSGI_OK;
}

#ifdef UWSGI_GO_CHEAP_CODE // Actually we only need uwsgi 2.0.1
static void fsmon_reload(struct uwsgi_fsmon *fs)
{
    qCDebug(CUTELYST_UWSGI) << "Reloading application due to file change";
    uwsgi_reload(uwsgi.argv);
}
#endif // UWSGI_GO_CHEAP_CODE

/**
 * This function is called when the master process is exiting
 */
extern "C" void uwsgi_cutelyst_master_cleanup()
{
    qCDebug(CUTELYST_UWSGI) << "Master process finishing" << QCoreApplication::applicationPid();
    delete qApp;
    qCDebug(CUTELYST_UWSGI) << "Master process finished" << QCoreApplication::applicationPid();
}

/**
 * This function is called when the child process is exiting
 */
extern "C" void uwsgi_cutelyst_atexit()
{
    qCDebug(CUTELYST_UWSGI) << "Child process finishing" << QCoreApplication::applicationPid();
    qCDebug(CUTELYST_UWSGI) << "Child process finished" << QCoreApplication::applicationPid();
}

extern "C" void uwsgi_cutelyst_init_apps()
{
    qCDebug(CUTELYST_UWSGI) << "Cutelyst Init App";

    QString path(options.app);
    if (path.isEmpty()) {
        qCCritical(CUTELYST_UWSGI) << "Cutelyst Application name or path was not set";
        return;
    }

#ifdef UWSGI_GO_CHEAP_CODE
    if (options.reload) {
        // Register application auto reload
        char *file = qstrdup(path.toUtf8().constData());
        uwsgi_register_fsmon(file, fsmon_reload, NULL);
    }
#endif // UWSGI_GO_CHEAP_CODE

    QString config(options.config);
    if (!config.isNull()) {
        qputenv("CUTELYST_CONFIG", config.toUtf8());
    }

    QPluginLoader *loader = new QPluginLoader(path);
    if (!loader->load()) {
        uwsgi_log("Could not load application: %s\n", loader->errorString().data());
        exit(1);
    }

    QObject *instance = loader->instance();
    if (!instance) {
        uwsgi_log("Could not get a QObject instance: %s\n", loader->errorString().data());
        exit(1);
    }

    Application *app = qobject_cast<Application *>(instance);
    if (!app) {
        uwsgi_log("Could not cast Cutelyst::Application from instance: %s\n", loader->errorString().data());
        exit(1);
    }

    MainThreadUWSGI *mainThread = 0;
    if (uwsgi.threads > 1) {
        for (int i = 0; i < uwsgi.threads; ++i) {
            coreThreads.append(new QThread);
        }
        mainThread = new MainThreadUWSGI;
    }

    for (int i = 0; i < uwsgi.async; ++i) {
        Application *coreApp = app;
        if (i > 0) {
            coreApp = qobject_cast<Application *>(app->metaObject()->newInstance());
            if (!coreApp) {
                uwsgi_log("*** FATAL *** Could not create a new instance of your Cutelyst::Application,"
                          "make sure your constructor has Q_INVOKABLE macro\n");
                exit(1);
            }
        }

        EngineUwsgi *engine = new EngineUwsgi;
        if (!engine->initApplication(coreApp, false)) {
            uwsgi_log("Failed to init application.\n");
            exit(1);
        }

        if (mainThread) {
            engine->setThread(coreThreads.at(i % coreThreads.size()));
            QObject::connect(engine, &EngineUwsgi::requestFinished,
                             mainThread, &MainThreadUWSGI::requestFinished, Qt::QueuedConnection);
        } else {
            QObject::connect(engine, &EngineUwsgi::requestFinished,
                             [=](wsgi_request *wsgi_req) {
                free_req_queue;
            });
        }

        coreEngines.append(engine);
    }

    qDebug() << "uwsgi.cores" << uwsgi.cores;

    // register a new app under a specific "mountpoint"
    uwsgi_add_app(1, CUTELYST_MODIFIER1, (char *) "", 0, NULL, NULL);
}

void uwsgi_cutelyst_watch_signal(int signalFD)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(signalFD, QSocketNotifier::Read);
    QObject::connect(socketNotifier, &QSocketNotifier::activated,
                     [=](int fd) {
        uwsgi_receive_signal(fd, (char *) "worker", uwsgi.mywid);
    });
}

void uwsgi_cutelyst_watch_request(struct uwsgi_socket *uwsgi_sock)
{
    QSocketNotifier *socketNotifier = new QSocketNotifier(uwsgi_sock->fd, QSocketNotifier::Read);
    QObject::connect(socketNotifier, &QSocketNotifier::activated,
                     [=](int fd) {
        struct wsgi_request *wsgi_req = find_first_available_wsgi_req();
        if (wsgi_req == NULL) {
            uwsgi_async_queue_is_full(uwsgi_now());
            return;
        }

        // fill wsgi_request structure
        wsgi_req_setup(wsgi_req, wsgi_req->async_id, uwsgi_sock);

        qCDebug(CUTELYST_UWSGI) << "wsgi_req->async_id" << wsgi_req->async_id << fd;
        qCDebug(CUTELYST_UWSGI) << "in_request" << uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request;

        // mark core as used
        uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 1;

        // accept the connection
        if (wsgi_req_simple_accept(wsgi_req, fd)) {
            uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request = 0;
            free_req_queue;
            return;
        }

        wsgi_req->start_of_request = uwsgi_micros();
        wsgi_req->start_of_request_in_sec = wsgi_req->start_of_request/1000000;

        // enter harakiri mode
        if (uwsgi.harakiri_options.workers > 0) {
            set_harakiri(uwsgi.harakiri_options.workers);
        }
        qCDebug(CUTELYST_UWSGI) << "in_request" << uwsgi.workers[uwsgi.mywid].cores[wsgi_req->async_id].in_request;

        Q_EMIT coreEngines.at(wsgi_req->async_id)->receiveRequest(wsgi_req);
        return;

//end:
//        uwsgi_close_request(wsgi_req);
//        free_req_queue;
    });
}

void uwsgi_cutelyst_loop()
{
    qDebug(CUTELYST_UWSGI) << "Using Cutelyst Qt Loop";

    // ensure SIGPIPE is ignored
    signal(SIGPIPE, SIG_IGN);

    // FIX for some reason this is not being set by UWSGI
    uwsgi.wait_read_hook = uwsgi_simple_wait_read_hook;

    if (!uwsgi.async_waiting_fd_table) {
        uwsgi.async_waiting_fd_table = static_cast<wsgi_request **>(uwsgi_calloc(sizeof(struct wsgi_request *) * uwsgi.max_fd));
    }

    if (!uwsgi.async_proto_fd_table) {
        uwsgi.async_proto_fd_table = static_cast<wsgi_request **>(uwsgi_calloc(sizeof(struct wsgi_request *) * uwsgi.max_fd));
    }

    // monitor signals
    if (uwsgi.signal_socket > -1) {
        uwsgi_cutelyst_watch_signal(uwsgi.signal_socket);
        uwsgi_cutelyst_watch_signal(uwsgi.my_signal_socket);
    }

    // monitor sockets
    struct uwsgi_socket *uwsgi_sock = uwsgi.sockets;
    while(uwsgi_sock) {
        uwsgi_cutelyst_watch_request(uwsgi_sock);
        uwsgi_sock = uwsgi_sock->next;
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
        uwsgi_log("%s[warn] %s\n", context.category, localMsg.constData());
        break;
    case QtCriticalMsg:
        uwsgi_log("%s[crit] %s\n", context.category, localMsg.constData());
        break;
    case QtFatalMsg:
        uwsgi_log("%s[fatal] %s\n", context.category, localMsg.constData());
        abort();
    }
}
