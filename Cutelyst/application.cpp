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
#include "controller.h"
#include "controller_p.h"
#include "response.h"
#include "response_p.h"
#include "dispatchtype.h"

#include "Actions/actionrest.h"
#include "Actions/roleacl.h"
#include "Actions/renderview.h"

#include <iostream>

#include <QStringList>

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

using namespace std;
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

    d->config = engine->config(QCoreApplication::applicationName());

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
        Q_FOREACH(Controller *controller, d->controllers) {
            controller->d_ptr->init(this, d->dispatcher);
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

    Context *ctx = new Context(priv);
    priv->response = new Response(ctx);
    priv->response->d_ptr->headers = d->headers;

    // Register application plugins
    Q_FOREACH (Plugin *plugin, d->plugins) {
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
