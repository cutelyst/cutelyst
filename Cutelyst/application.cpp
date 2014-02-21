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

#include "context_p.h"
#include "request.h"
#include "controller.h"
#include "response.h"

#include <iostream>

#include <QStringList>
#include <QDebug>

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

bool Application::registerPlugin(Plugin::AbstractPlugin *plugin)
{
    Q_D(Application);

    if (plugin->isApplicationPlugin()) {
        d->plugins << plugin;
        return true;
    }
    qWarning() << "The plugin:" << plugin->metaObject()->className() << "isn't an Application Plugin and cannot be registered";
    return false;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);

    if (d->engine) {
        qWarning() << "Tryied to register Controller after the Engine was setup, ignoring"
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
    Q_D(const Application);
    if (d->applicationName.isNull()) {
        return metaObject()->className();
    }
    return d->applicationName;
}

void Application::setApplicationName(const QByteArray &applicationName)
{
    Q_D(Application);
    d->applicationName = applicationName;
}

QByteArray Application::applicationVersion() const
{
    Q_D(const Application);
    return d->applicationVersion;
}

void Application::setApplicationVersion(const QByteArray &applicationVersion)
{
    Q_D(Application);
    d->applicationVersion = applicationVersion;
}

void Application::setup(Engine *engine)
{
    Q_D(Application);

    d->dispatcher->setupActions(d->controllers);
    d->engine = engine;
}

void Application::handleRequest(Request *req, Response *resp)
{
    Q_D(Application);

    ContextPrivate *priv = new ContextPrivate;
    priv->engine = d->engine;
    priv->dispatcher = d->dispatcher;
    priv->request = req;
    priv->response = resp;
    Context *ctx = new Context(priv);

    // Register application plugins
    foreach (Plugin::AbstractPlugin *plugin, d->plugins) {
        ctx->registerPlugin(plugin, false);
    }

    // Register context plugins
    registerPlugins(ctx);

    ctx->handleRequest();

    delete ctx;
}
