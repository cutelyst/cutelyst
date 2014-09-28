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

#include "context_p.h"

#include "common.h"
#include "request.h"
#include "response.h"
#include "action.h"
#include "dispatcher.h"
#include "controller.h"

#include <QUrl>
#include <QUrlQuery>

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

QVariantHash &Context::stash()
{
    Q_D(Context);
    return d->stash;
}

QUrl Context::uriFor(const QByteArray &path, const QStringList &args, const QMultiHash<QString, QString> &queryValues) const
{
    Q_D(const Context);

    const Action *action = d->dispatcher->getAction(path);
    return uriFor(action, args, queryValues);
}

QUrl Context::uriFor(const QByteArray &path, const QMultiHash<QString, QString> &queryValues) const
{
    return uriFor(path, QStringList(), queryValues);
}

QUrl Context::uriFor(const Action *action, const QStringList &args, const QMultiHash<QString, QString> &queryValues) const
{
    Q_D(const Context);

    QUrl ret = d->request->base();
    QByteArray path;

    if (action) {
        path = d->dispatcher->uriForAction(action, args);
    } else {
        // use the current action if none is provided
        path = d->dispatcher->uriForAction(d->action, args);
    }
    ret.setPath(path);

    QUrlQuery query;
    QMultiHash<QString, QString>::ConstIterator i = queryValues.constBegin();
    while (i != queryValues.constEnd()) {
        query.addQueryItem(i.key(), i.value());
        ++i;
    }
    ret.setQuery(query);

    return ret;
}

QUrl Context::uriFor(const Action *action, const QMultiHash<QString, QString> &queryValues) const
{
    return uriFor(action, QStringList(), queryValues);
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

bool Context::forward(const Action *action, const QStringList &arguments)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action, arguments);
}

bool Context::forward(const QByteArray &action, const QStringList &arguments)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action, arguments);
}

const Action *Context::getAction(const QByteArray &action, const QByteArray &ns)
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

ContextPrivate::~ContextPrivate()
{
    delete response;
}
