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

#include <iostream>

#include <QStringList>

Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER, "cutelyst.dispatcher")
Q_LOGGING_CATEGORY(CUTELYST_ENGINE, "cutelyst.engine")
Q_LOGGING_CATEGORY(CUTELYST_CORE, "cutelyst.core")
Q_LOGGING_CATEGORY(CUTELYST_UPLOAD, "cutelyst.upload")

using namespace std;
using namespace Cutelyst;

Application::Application(QObject *parent) :
    QObject(parent),
    d_ptr(new ApplicationPrivate)
{
    Q_D(Application);

    d->dispatcher = new Dispatcher(this);
}

Application::~Application()
{
    delete d_ptr;
}

bool Application::postFork()
{
    qCDebug(CUTELYST_CORE, "Default postFork called on pid: %lld", QCoreApplication::applicationPid());
    return true;
}

bool Application::registerPlugin(Plugin::AbstractPlugin *plugin)
{
    Q_D(Application);

    if (plugin->isApplicationPlugin()) {
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

    controller->setParent(this);
    d->controllers << controller;
    return true;
}

void Application::registerDispatcher(DispatchType *dispatcher)
{
    Q_D(const Application);
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

    d->engine = engine;
    d->config = engine->config(QLatin1String("Application"));

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
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
    priv->engine = d->engine;
    priv->dispatcher = d->dispatcher;
    priv->request = req;
    Context *ctx = new Context(priv);

    // Register application plugins
    Q_FOREACH (Plugin::AbstractPlugin *plugin, d->plugins) {
        ctx->registerPlugin(plugin, false);
    }

    // Register context plugins
    registerPlugins(ctx);

    ctx->handleRequest();

    delete ctx;
}
