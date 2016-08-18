/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "application_p.h"

#include "config.h"
#include "common.h"
#include "context_p.h"
#include "request.h"
#include "request_p.h"
#include "controller.h"
#include "controller_p.h"
#include "response.h"
#include "response_p.h"
#include "dispatchtype.h"
#include "view.h"
#include "stats.h"
#include "utils.h"

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QDataStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>

Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER, "cutelyst.dispatcher")
Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER_CHAINED, "cutelyst.dispatcher.chained")
Q_LOGGING_CATEGORY(CUTELYST_CONTROLLER, "cutelyst.controller")
Q_LOGGING_CATEGORY(CUTELYST_CORE, "cutelyst.core")
Q_LOGGING_CATEGORY(CUTELYST_ENGINE, "cutelyst.engine")
Q_LOGGING_CATEGORY(CUTELYST_UPLOAD, "cutelyst.upload")
Q_LOGGING_CATEGORY(CUTELYST_MULTIPART, "cutelyst.multipart")
Q_LOGGING_CATEGORY(CUTELYST_VIEW, "cutelyst.view")
Q_LOGGING_CATEGORY(CUTELYST_REQUEST, "cutelyst.request")
Q_LOGGING_CATEGORY(CUTELYST_RESPONSE, "cutelyst.response")
Q_LOGGING_CATEGORY(CUTELYST_STATS, "cutelyst.stats")
Q_LOGGING_CATEGORY(CUTELYST_COMPONENT, "cutelyst.component")

using namespace Cutelyst;

Application::Application(QObject *parent) :
    QObject(parent),
    d_ptr(new ApplicationPrivate)
{
    Q_D(Application);

    d->q_ptr = this;
    d->headers.setHeader(QStringLiteral("X-Cutelyst"), QStringLiteral(VERSION));

    qRegisterMetaType<ParamsMultiMap>();
    qRegisterMetaTypeStreamOperators<ParamsMultiMap>("ParamsMultiMap");

    d->dispatcher = new Dispatcher(this);
}

Application::~Application()
{
    delete d_ptr;
}

bool Application::init()
{
    qCDebug(CUTELYST_CORE) << "Default Application::init called on pid:" << QCoreApplication::applicationPid();
    return true;
}

bool Application::postFork()
{
    qCDebug(CUTELYST_CORE) << "Default Application::postFork called on pid:" << QCoreApplication::applicationPid();
    return true;
}

Headers &Application::defaultHeaders()
{
    Q_D(Application);
    return d->headers;
}

bool Application::registerPlugin(Plugin *plugin)
{
    Q_D(Application);
    if (d->plugins.contains(plugin)) {
        return false;
    }
    d->plugins.append(plugin);
    return true;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);
    auto name = QString::fromLatin1(controller->metaObject()->className());
    if (d->controllersHash.contains(name)) {
        return false;
    }
    d->controllersHash.insert(name, controller);
    d->controllers.append(controller);
    return true;
}

bool Application::registerView(View *view)
{
    Q_D(Application);
    if (d->views.contains(view->name())) {
        qCWarning(CUTELYST_CORE) << "Not registering View." << view->metaObject()->className()
                                 << "There is already a view with this name:" << view->name();
        return false;
    }
    d->views.insert(view->name(), view);
    return true;
}

bool Application::registerDispatcher(DispatchType *dispatcher)
{
    Q_D(Application);
    if (d->dispatchers.contains(dispatcher)) {
        return false;
    }
    d->dispatchers.append(dispatcher);
    return true;
}

Component *Application::createComponentPlugin(const QString &name, QObject *parent)
{
    Q_D(Application);
    auto it = d->factories.constFind(name);
    if (it != d->factories.constEnd()) {
        ComponentFactory *factory = it.value();
        if (factory) {
            return factory->createComponent(parent);
        } else {
            return 0;
        }
    }

    QDir pluginsDir(QLatin1String(CUTELYST_PLUGINS_DIR));
    QPluginLoader loader;
    Component *component = nullptr;
    ComponentFactory *factory = nullptr;
    const auto plugins = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : plugins) {
        loader.setFileName(pluginsDir.absoluteFilePath(fileName));
        const QJsonObject json = loader.metaData()[QLatin1String("MetaData")].toObject();
        if (json[QLatin1String("name")].toString() == name) {
            QObject *plugin = loader.instance();
            if (plugin) {
                factory = qobject_cast<ComponentFactory *>(plugin);
                if (!factory) {
                    qCCritical(CUTELYST_CORE) << "Could not create a factory for" << loader.fileName();
                } else {
                    component = factory->createComponent(parent);
                }
                break;
            } else {
                qCCritical(CUTELYST_CORE) << "Could not load plugin" << loader.fileName() << loader.errorString();
            }
        }
    }
    d->factories.insert(name, factory);

    return component;
}

const char *Application::cutelystVersion()
{
    return VERSION;
}

QVector<Cutelyst::Controller *> Application::controllers() const
{
    Q_D(const Application);
    return d->controllers;
}

View *Application::view(const QString &name) const
{
    Q_D(const Application);
    return d->views.value(name);
}

QVariant Application::config(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Application);
    auto it = d->config.constFind(key);
    if (it != d->config.constEnd()) {
        return it.value();
    }
    return defaultValue;
}

Dispatcher *Application::dispatcher() const
{
    Q_D(const Application);
    return d->dispatcher;
}

QVector<Cutelyst::DispatchType *> Application::dispatchers() const
{
    Q_D(const Application);
    return d->dispatcher->dispatchers();
}

QVariantMap Application::config() const
{
    Q_D(const Application);
    return d->config;
}

QString Cutelyst::Application::pathTo(const QStringList &path) const
{
    QDir home = config(QStringLiteral("home")).toString();
    return home.absoluteFilePath(path.join(QLatin1Char('/')));
}

bool Cutelyst::Application::inited() const
{
    Q_D(const Application);
    return d->init;
}

Cutelyst::Engine *Cutelyst::Application::engine() const
{
    Q_D(const Application);
    return d->engine;
}

void Application::setConfig(const QString &key, const QVariant &value)
{
    Q_D(Application);
    d->config.insert(key, value);
}

bool Application::setup(Engine *engine)
{
    Q_D(Application);

    if (d->init) {
        return true;
    }
    d->init = true;

    d->useStats = CUTELYST_STATS().isDebugEnabled();
    d->engine = engine;
    d->config = engine->config(QLatin1String("Cutelyst"));

    d->setupHome();

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
        d->setupChildren(children());

        bool zeroCore = engine->workerCore() == 0;

        QVector<QStringList> tablePlugins;
        const auto plugins = d->plugins;
        for (Plugin *plugin : plugins) {
            if (plugin->objectName().isEmpty()) {
                plugin->setObjectName(QString::fromLatin1(plugin->metaObject()->className()));
            }
            tablePlugins.append({ plugin->objectName() });
            // Configure plugins
            plugin->setup(this);
        }

        if (zeroCore && !tablePlugins.isEmpty()) {
            qCDebug(CUTELYST_CORE) << Utils::buildTable(tablePlugins, QStringList(),
                                                        QLatin1String("Loaded plugins:")).constData();
        }

        if (zeroCore) {
            QVector<QStringList> tableDataHandlers;
            tableDataHandlers.append({ QLatin1String("application/x-www-form-urlencoded") });
            tableDataHandlers.append({ QLatin1String("application/json") });
            tableDataHandlers.append({ QLatin1String("multipart/form-data") });
            qCDebug(CUTELYST_CORE) << Utils::buildTable(tableDataHandlers, QStringList(),
                                                        QLatin1String("Loaded Request Data Handlers:")).constData();

            qCDebug(CUTELYST_CORE) << "Loaded dispatcher" << QString::fromLatin1(d->dispatcher->metaObject()->className());
            qCDebug(CUTELYST_CORE) << "Using engine" << QString::fromLatin1(d->engine->metaObject()->className());
        }

        QString home = d->config.value(QLatin1String("home")).toString();
        if (home.isEmpty()) {
            if (zeroCore) {
                qCDebug(CUTELYST_CORE) << "Couldn't find home";
            }
        } else {
            QFileInfo homeInfo = home;
            if (homeInfo.isDir()) {
                if (zeroCore) {
                    qCDebug(CUTELYST_CORE) << "Found home" << home;
                }
            } else {
                if (zeroCore) {
                    qCDebug(CUTELYST_CORE) << "Home" << home << "doesn't exist";
                }
            }
        }

        QVector<QStringList> table;
        QStringList controllerNames = d->controllersHash.keys();
        controllerNames.sort();
        for (const QString controller : controllerNames) {
            table.append({ controller, QLatin1String("Controller")});
        }

        const auto views = d->views;
        for (View *view : views) {
            if (view->reverse().isEmpty()) {
                const QString className = QString::fromLatin1(view->metaObject()->className()) + QLatin1String("->execute");
                view->setReverse(className);
            }
            table.append({ view->reverse(), QLatin1String("View")});
        }

        if (zeroCore && !table.isEmpty()) {
            qCDebug(CUTELYST_CORE) << Utils::buildTable(table, {
                                                            QLatin1String("Class"), QLatin1String("Type")
                                                        },
                                                        QLatin1String("Loaded components:")).constData();
        }

        for (Controller *controller : d->controllers) {
            controller->d_ptr->init(this, d->dispatcher);
        }

        d->dispatcher->setupActions(d->controllers, d->dispatchers, d->engine->workerCore() == 0);

        if (zeroCore) {
            qCInfo(CUTELYST_CORE) << QString::fromLatin1("%1 powered by Cutelyst %2, Qt %3.")
                                     .arg(QCoreApplication::applicationName(), QLatin1String(Application::cutelystVersion()), QLatin1String(qVersion()))
                                     .toLatin1().constData();
        }

        Q_EMIT preForked(this);

        return true;
    }

    return false;
}

void Application::handleRequest(Request *req)
{
    Q_D(Application);

    Stats *stats = nullptr;
    if (d->useStats) {
        stats = new Stats(this);
    }

    Context *c = new Context(this,
                             d->engine,
                             d->dispatcher,
                             req->d_ptr->requestPtr,
                             req,
                             d->plugins,
                             stats,
                             d->headers);

    // Process request
    bool skipMethod = false;
    Q_EMIT beforePrepareAction(c, &skipMethod);

    if (!skipMethod) {
        static bool log = CUTELYST_REQUEST().isEnabled(QtDebugMsg);
        if (log) {
            d->logRequest(req);
        }

        d->dispatcher->prepareAction(c);

        Q_EMIT beforeDispatch(c);

        d->dispatcher->dispatch(c);

        Q_EMIT afterDispatch(c);
    }

    d->engine->finalize(c);

    if (stats) {
        qCDebug(CUTELYST_STATS, "Response Code: %d; Content-Type: %s; Content-Length: %s",
                c->response()->status(),
                c->response()->headers().header(QStringLiteral("Content-Type"), QStringLiteral("unknown")).toLatin1().data(),
                c->response()->headers().header(QStringLiteral("Content-Length"), QStringLiteral("unknown")).toLatin1().data());

        RequestPrivate *reqPriv = req->d_ptr;
        reqPriv->endOfRequest = d->engine->time();
        double enlapsed = (reqPriv->endOfRequest - reqPriv->startOfRequest) / 1000000.0;
        QString average;
        if (enlapsed == 0.0) {
            average = QStringLiteral("??");
        } else {
            average = QString::number(1.0 / enlapsed, 'f');
            average.truncate(average.size() - 3);
        }
        qCInfo(CUTELYST_STATS) << QStringLiteral("Request took: %1s (%2/s)\n%3")
                                  .arg(QString::number(enlapsed, 'f'), average, QString::fromLatin1(stats->report()))
                                  .toLatin1().constData();
        delete stats;
    }

    delete c;
}

bool Application::enginePostFork()
{
    Q_D(Application);

    if (!postFork()) {
        return false;
    }

    for (Controller *controller : d->controllers) {
        if (!controller->postFork(this)) {
            return false;
        }
    }

    Q_EMIT postForked(this);

    return true;
}


void Cutelyst::ApplicationPrivate::setupHome()
{
    // Hook the current directory in config if "home" is not set
    if (!config.contains(QLatin1String("home"))) {
        config.insert(QStringLiteral("home"), QDir::currentPath());
    }

    if (!config.contains(QLatin1String("root"))) {
        QDir home = config.value(QLatin1String("home")).toString();
        config.insert(QStringLiteral("root"), home.absoluteFilePath(QLatin1String("root")));
    }
}

void ApplicationPrivate::setupChildren(const QObjectList &children)
{
    Q_Q(Application);
    for (QObject *child : children) {
        Controller *controller = qobject_cast<Controller *>(child);
        if (controller) {
            q->registerController(controller);
            continue;
        }

        Plugin *plugin = qobject_cast<Plugin *>(child);
        if (plugin) {
            q->registerPlugin(plugin);
            continue;
        }

        View *view = qobject_cast<View *>(child);
        if (view) {
            q->registerView(view);
            continue;
        }

        DispatchType *dispatchType = qobject_cast<DispatchType *>(child);
        if (dispatchType) {
            q->registerDispatcher(dispatchType);
            continue;
        }
    }
}

void Cutelyst::ApplicationPrivate::logRequest(Request *req)
{
    QString path = req->path();
    if (path.isEmpty()) {
        path = QStringLiteral("/");
    }
    qCDebug(CUTELYST_REQUEST) << req->method() << "request for" << path << "from" << req->address().toString();

    ParamsMultiMap params = req->queryParameters();
    if (!params.isEmpty()) {
        logRequestParameters(params, QStringLiteral("Query Parameters are:"));
    }

    params = req->bodyParameters();
    if (!params.isEmpty()) {
        logRequestParameters(params, QStringLiteral("Body Parameters are:"));
    }

    const auto uploads = req->uploads();
    if (!uploads.isEmpty()) {
        logRequestUploads(uploads);
    }
}

void Cutelyst::ApplicationPrivate::logRequestParameters(const ParamsMultiMap &params, const QString &title)
{
    QVector<QStringList> table;
    auto it = params.constBegin();
    while (it != params.constEnd()) {
        table.append({ it.key(), it.value() });
        ++it;
    }
    qCDebug(CUTELYST_REQUEST) << Utils::buildTable(table, {
                                                       QStringLiteral("Parameter"),
                                                       QStringLiteral("Value"),
                                                   },
                                                   title).constData();
}

void Cutelyst::ApplicationPrivate::logRequestUploads(const QVector<Cutelyst::Upload *> &uploads)
{
    QVector<QStringList> table;
    for (Upload *upload : uploads) {
        table.append({ upload->name(),
                       upload->filename(),
                       upload->contentType(),
                       QString::number(upload->size())
                     });
    }
    qCDebug(CUTELYST_REQUEST) << Utils::buildTable(table, {
                                                       QStringLiteral("Parameter"),
                                                       QStringLiteral("Filename"),
                                                       QStringLiteral("Type"),
                                                       QStringLiteral("Size"),
                                                   },
                                                   QStringLiteral("File Uploads are:")).constData();
}

#include "moc_application.cpp"
