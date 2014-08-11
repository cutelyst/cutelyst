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

#include "context_p.h"

#include "common.h"
#include "engine.h"
#include "request.h"
#include "response.h"
#include "action.h"
#include "dispatcher.h"
#include "controller.h"

#include <QUrl>
#include <QStringBuilder>
#include <QStringList>
#include <QTime>

using namespace Cutelyst;

Context::Context(ContextPrivate *priv) :
    d_ptr(priv)
{
}

Context::~Context()
{
    delete d_ptr;
}

bool Context::error() const
{
    Q_D(const Context);
    return !d->error.isEmpty();
}

void Context::error(const QString &error)
{
    Q_D(Context);
    if (!error.isEmpty()) {
        d->error << error;
    }
}

QStringList Context::errors() const
{
    Q_D(const Context);
    return d->error;
}

bool Context::state() const
{
    Q_D(const Context);
    return d->state;
}

void Context::setState(bool state)
{
    Q_D(Context);
    d->state = state;
}

Engine *Context::engine() const
{
    Q_D(const Context);
    return d->engine;
}

Application *Context::app() const
{
    Q_D(const Context);
    return d->app;
}

Response *Context::response() const
{
    Q_D(const Context);
    return d->response;
}

Response *Context::res() const
{
    Q_D(const Context);
    return d->response;
}

Action *Context::action() const
{
    Q_D(const Context);
    return d->action;
}

QByteArray Context::actionName() const
{
    Q_D(const Context);
    return d->action->name();
}

Request *Context::request() const
{
    Q_D(const Context);
    return d->request;
}

Request *Context::req() const
{
    Q_D(const Context);
    return d->request;
}

Dispatcher *Context::dispatcher() const
{
    Q_D(const Context);
    return d->dispatcher;
}

QByteArray Cutelyst::Context::controllerName() const
{
    Q_D(const Context);
    return d->action->controller()->metaObject()->className();
}

Controller *Context::controller(const QByteArray &name) const
{
    Q_D(const Context);
    if (name.isEmpty()) {
        return d->action->controller();
    } else {
        return d->dispatcher->controllers().value(name);
    }
}

QString Context::match() const
{
    Q_D(const Context);
    return d->match;
}

QVariantHash &Context::stash()
{
    Q_D(Context);
    return d->stash;
}

QByteArray Context::uriFor(const QByteArray &path, const QStringList &args)
{
    Q_D(Context);

    Action *action = d->dispatcher->getAction(path);
    if (action) {
        return d->dispatcher->uriForAction(action, args);
    } else {
        return path;
    }
}

bool Context::dispatch()
{
    Q_D(Context);
    return d->dispatcher->dispatch(this);
}

bool Context::detached() const
{
    Q_D(const Context);
    return d->detached;
}

void Context::detach()
{
    Q_D(Context);
    d->detached = true;
}

bool Context::forward(const QByteArray &action, const QStringList &arguments)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action, arguments);
}

Action *Context::getAction(const QByteArray &action, const QByteArray &ns)
{
    Q_D(Context);
    return d->dispatcher->getAction(action, ns);
}

QList<Action *> Context::getActions(const QByteArray &action, const QByteArray &ns)
{
    Q_D(Context);
    return d->dispatcher->getActions(action, ns);
}

bool Context::registerPlugin(Plugin::AbstractPlugin *plugin, bool takeOwnership)
{
    Q_D(Context);
    if (plugin->setup(this)) {
        if (takeOwnership) {
            plugin->setParent(this);

            if (plugin->isApplicationPlugin()) {
                qCWarning(CUTELYST_CORE) << "The plugin:" << plugin->metaObject()->className() << "is an Application plugin and should be registered there";
            }
        }
        d->plugins.insert(plugin, QVariantHash());
        return true;
    }
    return false;
}

QList<Plugin::AbstractPlugin *> Context::plugins()
{
    Q_D(Context);
    return d->plugins.keys();
}

void Context::handleRequest()
{
    Q_D(Context);

    bool skipMethod = false;
    Q_EMIT beforePrepareAction(&skipMethod);
    if (!skipMethod) {
        d->dispatcher->prepareAction(this);

        Q_EMIT beforeDispatch();
        dispatch();
        Q_EMIT afterDispatch();
    }

    d->status = finalize();
}

int Context::finalize()
{
    Q_D(Context);

    if (error()) {
        d->engine->finalizeError(this);
    }

    Response *response = d->response;

    if (response->location().isValid()) {
        response->addHeaderValue(QByteArrayLiteral("Location"), response->location().toEncoded());

        if (!response->hasBody()) {
            QByteArray data;
            data = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                   "  <head>\n"
                   "    <title>Moved</title>\n"
                   "  </head>\n"
                   "  <body>\n"
                   "     <p>This item has moved <a href=";
            data.append(response->location().toEncoded());
            data.append(">here</a>.</p>\n"
                   "  </body>\n"
                   "</html>\n");
            response->body() = data;
            response->setContentType("text/html; charset=utf-8");
        }
    }

    d->engine->finalizeCookies(this, d->request->engineData());

    QIODevice *body = response->bodyDevice();
    if (body) {
        response->setContentLength(body->size());
    }

    d->engine->finalizeHeaders(this, d->request->engineData());

    if (body) {
        d->engine->finalizeBody(this, body, d->request->engineData());
    }

    if (d->stats) {
        qCDebug(CUTELYST_CORE) << "Request took:" << d->stats->elapsed() / 1000.0 << "s";
    }

    return response->status();
}

QVariant Context::pluginProperty(Plugin::AbstractPlugin * const plugin, const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Context);
    return d->plugins.value(plugin).value(key, defaultValue);
}

void Context::setPluginProperty(Plugin::AbstractPlugin *plugin, const QString &key, const QVariant &value)
{
    Q_D(Context);
    d->plugins[plugin].insert(key, value);
}

ContextPrivate::ContextPrivate() :
    response(new Response)
{
    stats = new QTime;
    stats->start();
}

ContextPrivate::~ContextPrivate()
{
    delete response;
}
