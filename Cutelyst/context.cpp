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

QStringList Context::args() const
{
    Q_D(const Context);
    return d->request->args();
}

Engine *Context::engine() const
{
    Q_D(const Context);
    return d->engine;
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

QString Context::actionName() const
{
    Q_D(const Context);
    return d->action->name();
}

QString Context::ns() const
{
    Q_D(const Context);
    return d->action->ns();
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

QString Cutelyst::Context::controllerName() const
{
    Q_D(const Context);
    return d->action->controller()->metaObject()->className();
}

Controller *Context::controller(const QString &name) const
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

QString Context::uriFor(const QString &path, const QStringList &args)
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

bool Context::forward(const QString &action, const QStringList &arguments)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action, arguments);
}

Action *Context::getAction(const QString &action, const QString &ns)
{
    Q_D(Context);
    return d->dispatcher->getAction(action, ns);
}

QList<Action *> Context::getActions(const QString &action, const QString &ns)
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
    beforePrepareAction(&skipMethod);
    if (!skipMethod) {
        prepareAction();
        afterPrepareAction();

        beforeDispatch();
        dispatch();
        afterDispatch();
    }

    d->status = finalize();
}

void Context::prepareAction()
{
    Q_D(Context);
    d->dispatcher->prepareAction(this);
}

void Context::finalizeHeaders()
{
    Q_D(Context);

    Response *response = d->response;
    if (response->finalizedHeaders()) {
        return;
    }

    if (response->location().isValid()) {
        response->addHeaderValue("Location", response->location().toEncoded());

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

    if (response->hasBody()) {
        response->setContentLength(response->body().size());
    }

    finalizeCookies();

    d->engine->finalizeHeaders(this);
}

void Context::finalizeCookies()
{
    Q_D(Context);
    d->engine->finalizeCookies(this);
}

void Context::finalizeBody()
{
    Q_D(Context);
    d->engine->finalizeBody(this);
}

void Context::finalizeError()
{
    Q_D(Context);
    d->engine->finalizeError(this);
}

int Context::finalize()
{
    Q_D(Context);

    if (error()) {
        finalizeError();
    }

    finalizeHeaders();

    if (d->request->method() == "HEAD") {
        d->response->body().clear();
    }

    finalizeBody();

    if (d->stats) {
        qCDebug(CUTELYST_CORE, "Request took: %5.3fs", d->stats->elapsed() / 1000.0);
    }

    return d->response->status();
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
    response(new Response),
    action(0),
    detached(false),
    state(false),
    stats(0)
{
    stats = new QTime;
    stats->start();
}

ContextPrivate::~ContextPrivate()
{
    delete response;
}
