/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "Actions/actionrest.h"
#include "Actions/roleacl.h"
#include "Actions/renderview.h"

#include <QDir>
#include <QStringList>
#include <QStringBuilder>

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
Q_LOGGING_CATEGORY(CUTELYST_CODE, "cutelyst.code")

using namespace Cutelyst;

Application::Application(QObject *parent) :
    QObject(parent),
    d_ptr(new ApplicationPrivate)
{
    Q_D(Application);

    d->headers.setHeader(QStringLiteral("X-Cutelyst"), QStringLiteral(VERSION));

    qRegisterMetaType<ParamsMultiMap>();
    qRegisterMetaType<ActionREST *>();
    qRegisterMetaType<RoleACL *>();
    qRegisterMetaType<RenderView *>();

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

bool Application::registerPlugin(Cutelyst::Plugin *plugin)
{
    Q_D(Application);

    d->plugins.append(plugin);
    return true;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);

    if (d->init) {
        qCWarning(CUTELYST_CORE) << "Tryied to register Controller after the Application was initted, ignoring"
                                 << controller->metaObject()->className();
        return false;
    }

    d->controllers.append(controller);
    return true;
}

QList<Controller *> Application::controllers() const
{
    Q_D(const Application);
    return d->controllers;
}

bool Application::registerView(View *view, const QString &name)
{
    Q_D(Application);
    if (d->views.contains(name)) {
        qCWarning(CUTELYST_CORE) << "Not registering View. There is already a view with this name:" << name;
        return false;
    }
    d->views.insert(name, view);
    return true;
}

View *Application::view(const QString &name) const
{
    Q_D(const Application);
    return d->views.value(name);
}

void Application::registerDispatcher(DispatchType *dispatcher)
{
    Q_D(const Application);
    dispatcher->setParent(this);
    d->dispatcher->registerDispatchType(dispatcher);
}

QVariant Application::config(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Application);
    QVariantHash::const_iterator it = d->config.constFind(key);
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

QList<DispatchType *> Application::dispatchers() const
{
    Q_D(const Application);
    return d->dispatcher->dispatchers();
}

QVariantHash Application::config() const
{
    Q_D(const Application);
    return d->config;
}

QString Cutelyst::Application::pathTo(const QStringList &path) const
{
    QDir home = config(QStringLiteral("home")).toString();
    return home.absoluteFilePath(path.join(QDir::separator()));
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

    d->useStats = CUTELYST_STATS().isDebugEnabled();
    d->engine = engine;
    d->config = engine->config(QStringLiteral("CUTELYST"));

    d->setupHome();

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
        QString appName = QCoreApplication::applicationName();

        QList<QStringList> tablePlugins;
        Q_FOREACH (Plugin *plugin, d->plugins) {
            QString className = QString::fromLatin1(plugin->metaObject()->className());
            tablePlugins.append({ className });
            // Configure plugins
            plugin->setup(this);
        }
        qCDebug(CUTELYST_CORE) << DispatchType::buildTable(tablePlugins, QStringList(),
                                                           QStringLiteral("Loaded plugins:")).data();

        qCDebug(CUTELYST_CORE) << "Loaded dispatcher" << QString::fromLatin1(d->dispatcher->metaObject()->className());
        qCDebug(CUTELYST_CORE) << "Using engine" << QString::fromLatin1(d->engine->metaObject()->className());

        QString home = d->config.value("home").toString();
        if (home.isEmpty()) {
            qCDebug(CUTELYST_CORE) << "Couldn't find home";
        } else {
            QFileInfo homeInfo = home;
            if (homeInfo.isDir()) {
                qCDebug(CUTELYST_CORE) << "Found home" << home;
            } else {
                qCDebug(CUTELYST_CORE) << "Home" << home << "doesn't exist";
            }
        }

        QList<QStringList> table;
        Q_FOREACH (Controller *controller, d->controllers) {
            QString className = QString::fromLatin1(controller->metaObject()->className());
            if (!className.startsWith(QLatin1String("Cutelyst"))) {
                className = appName % QLatin1String("::Controller::") % className;
            }
            table.append({ className, QStringLiteral("instance")});
        }

        Q_FOREACH (View *view, d->views) {
            QString className = QString::fromLatin1(view->metaObject()->className());
            if (!className.startsWith(QLatin1String("Cutelyst"))) {
                className = appName % QLatin1String("::View::") % className;
            }
            table.append({ className, QStringLiteral("instance")});
        }

        qCDebug(CUTELYST_CORE) << DispatchType::buildTable(table, {
                                                               QStringLiteral("Class"), QStringLiteral("Type")
                                                           },
                                                           QStringLiteral("Loaded components:")).data();

        Q_FOREACH (Controller *controller, d->controllers) {
            controller->d_ptr->init(this, d->dispatcher);
        }

        d->dispatcher->setupActions(d->controllers);
        d->init = true;

        qCDebug(CUTELYST_CORE) << QString("%1 powered by Cutelyst %2")
                                  .arg(QCoreApplication::applicationName())
                                  .arg(VERSION).toLatin1().data();
        return true;
    }

    return false;
}

void Application::handleRequest(Request *req)
{
    Q_D(Application);

    ContextPrivate *priv = new ContextPrivate;
    priv->app = this;
    priv->engine = req->engine();
    priv->dispatcher = d->dispatcher;
    priv->request = req;
    priv->plugins = d->plugins;

    Context *ctx = new Context(priv);
    priv->response = new Response(ctx);
    priv->response->d_ptr->headers = d->headers;

    if (d->useStats) {
        priv->stats = new Stats(this);
    }

    // Process request
    bool skipMethod = false;
    Q_EMIT beforePrepareAction(ctx, &skipMethod);
    if (!skipMethod) {
        d->dispatcher->prepareAction(ctx);

        Q_EMIT beforeDispatch(ctx);

        d->dispatcher->dispatch(ctx);

        Q_EMIT afterDispatch(ctx);
    }
    priv->engine->finalize(ctx);

    if (priv->stats) {
        qCDebug(CUTELYST_STATS, "Response Code: %d; Content-Type: %s; Content-Length: %lld",
                ctx->response()->status(),
                ctx->response()->contentType().toLatin1().data(),
                ctx->response()->contentLength());

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
        qCDebug(CUTELYST_STATS) << QString("Request took: %1s (%2/s)\n%3")
                                   .arg(QString::number(enlapsed, 'f'))
                                   .arg(average)
                                   .arg(priv->stats->report())
                                   .toLatin1().data();
        delete priv->stats;
    }

    delete ctx;
}

bool Application::enginePostFork()
{
    Q_D(Application);

    if (!postFork()) {
        return false;
    }

    Q_FOREACH (Controller *controller, d->controllers) {
        if (!controller->postFork(this)) {
            return false;
        }
    }
    return true;
}


void Cutelyst::ApplicationPrivate::setupHome()
{
    // Hook the current directory in config if "home" is not set
    if (!config.contains("home")) {
        config.insert("home", QDir::currentPath());
    }

    if (!config.contains("root")) {
        QDir home = config.value("home").toString();
        config.insert("root", home.absoluteFilePath("root"));
    }
}
