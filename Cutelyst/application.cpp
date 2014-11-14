/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "common.h"
#include "context_p.h"
#include "request.h"
#include "controller.h"
#include "response.h"
#include "dispatchtype.h"

#include "Actions/actionrest.h"
#include "Actions/roleacl.h"
#include "Actions/renderview.h"

#include <iostream>

#include <QStringList>

Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER, "cutelyst.dispatcher")
Q_LOGGING_CATEGORY(CUTELYST_CONTROLLER, "cutelyst.controller")
Q_LOGGING_CATEGORY(CUTELYST_CORE, "cutelyst.core")
Q_LOGGING_CATEGORY(CUTELYST_ENGINE, "cutelyst.engine")
Q_LOGGING_CATEGORY(CUTELYST_UPLOAD, "cutelyst.upload")
Q_LOGGING_CATEGORY(CUTELYST_MULTIPART, "cutelyst.multipart")
Q_LOGGING_CATEGORY(CUTELYST_VIEW, "cutelyst.view")
Q_LOGGING_CATEGORY(CUTELYST_REQUEST, "cutelyst.request")
Q_LOGGING_CATEGORY(CUTELYST_RESPONSE, "cutelyst.response")
Q_LOGGING_CATEGORY(CUTELYST_ENGINE_HTTP, "cutelyst.engine.http")
Q_LOGGING_CATEGORY(CUTELYST_STATS, "cutelyst.stats")

using namespace std;
using namespace Cutelyst;

Application::Application(QObject *parent) :
    QObject(parent),
    d_ptr(new ApplicationPrivate)
{
    Q_D(Application);

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

bool Application::registerPlugin(Plugin::AbstractPlugin *plugin)
{
    Q_D(Application);

    if (plugin->isApplicationPlugin()) {
        plugin->setParent(this);
        d->plugins << plugin;
        return true;
    }
    qCWarning(CUTELYST_CORE) << "The plugin:" << plugin->metaObject()->className() << "isn't an Application Plugin and cannot be registered";
    return false;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);

    if (d->init) {
        qCWarning(CUTELYST_CORE) << "Tryied to register Controller after the Application was initted, ignoring"
                                 << controller->metaObject()->className();
        return false;
    }

    d->controllers << controller;
    return true;
}

bool Application::registerView(View *view, const QByteArray &name)
{
    Q_D(Application);
    if (d->views.contains(name)) {
        return false;
    }
    d->views.insert(name, view);
    return true;
}

View *Application::view(const QByteArray &name) const
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

QByteArray Application::applicationName() const
{
    return metaObject()->className();
}

QByteArray Application::applicationVersion() const
{
    return QCoreApplication::applicationVersion().toLocal8Bit();
}

QVariantHash Application::config() const
{
    Q_D(const Application);
    return d->config;
}

bool Application::setup(Engine *engine)
{
    Q_D(Application);

    if (d->init) {
        return true;
    }

    d->config = engine->config(QLatin1String("Application"));

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
        Q_FOREACH(Controller *controller, d->controllers) {
            controller->init(this);
        }

        d->dispatcher->setupActions(d->controllers);
        d->init = true;
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
    priv->response = new Response;
    Context *ctx = new Context(priv);

    // Register application plugins
    Q_FOREACH (Plugin::AbstractPlugin *plugin, d->plugins) {
        ctx->registerPlugin(plugin, false);
    }

    // Register context plugins
    Q_EMIT registerPlugins(ctx);

    // Process request
    bool skipMethod = false;
    Q_EMIT ctx->beforePrepareAction(&skipMethod);
    if (!skipMethod) {
        d->dispatcher->prepareAction(ctx);

        Q_EMIT ctx->beforeDispatch();

        d->dispatcher->dispatch(ctx);

        Q_EMIT ctx->afterDispatch();
    }
    priv->engine->finalize(ctx);

    delete ctx;
}
