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

#include "cutelyst_p.h"

#include "cutelystengine.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "cutelystaction.h"
#include "cutelystdispatcher.h"

#include <QUrl>
#include <QStringBuilder>
#include <QStringList>
#include <QTime>

Cutelyst::Cutelyst(CutelystPrivate *priv) :
    d_ptr(priv)
{
}

Cutelyst::~Cutelyst()
{
    delete d_ptr;
}

bool Cutelyst::error() const
{
    Q_D(const Cutelyst);
    return !d->error.isEmpty();
}

void Cutelyst::error(const QString &error)
{
    Q_D(Cutelyst);
    if (!error.isEmpty()) {
        d->error << error;
    }
}

QStringList Cutelyst::errors() const
{
    Q_D(const Cutelyst);
    return d->error;
}

bool Cutelyst::state() const
{
    Q_D(const Cutelyst);
    return d->state;
}

void Cutelyst::setState(bool state)
{
    Q_D(Cutelyst);
    d->state = state;
}

QStringList Cutelyst::args() const
{
    Q_D(const Cutelyst);
    return d->request->args();
}

CutelystEngine *Cutelyst::engine() const
{
    Q_D(const Cutelyst);
    return d->engine;
}

CutelystResponse *Cutelyst::response() const
{
    Q_D(const Cutelyst);
    return d->response;
}

CutelystResponse *Cutelyst::res() const
{
    Q_D(const Cutelyst);
    return d->response;
}

CutelystAction *Cutelyst::action() const
{
    Q_D(const Cutelyst);
    return d->action;
}

QString Cutelyst::ns() const
{
    Q_D(const Cutelyst);
    return d->action->ns();
}

CutelystRequest *Cutelyst::request() const
{
    Q_D(const Cutelyst);
    return d->request;
}

CutelystRequest *Cutelyst::req() const
{
    Q_D(const Cutelyst);
    return d->request;
}

CutelystDispatcher *Cutelyst::dispatcher() const
{
    Q_D(const Cutelyst);
    return d->dispatcher;
}

CutelystController *Cutelyst::controller(const QString &name) const
{
    Q_D(const Cutelyst);
    if (name.isEmpty()) {
        return d->action->controller();
    } else {
        return d->dispatcher->controllers().value(name);
    }
}

QString Cutelyst::match() const
{
    Q_D(const Cutelyst);
    return d->match;
}

QVariantHash &Cutelyst::stash()
{
    Q_D(Cutelyst);
    return d->stash;
}

QString Cutelyst::uriFor(const QString &path, const QStringList &args)
{
    Q_D(Cutelyst);

    CutelystAction *action = d->dispatcher->getAction(path);
    if (action) {
        return d->dispatcher->uriForAction(action, args);
    } else {
        return path;
    }
}

bool Cutelyst::dispatch()
{
    Q_D(Cutelyst);
    return d->dispatcher->dispatch(this);
}

bool Cutelyst::detached() const
{
    Q_D(const Cutelyst);
    return d->detached;
}

void Cutelyst::detach()
{
    Q_D(Cutelyst);
    d->detached = true;
}

bool Cutelyst::forward(const QString &action, const QStringList &arguments)
{
    Q_D(Cutelyst);
    return d->dispatcher->forward(this, action, arguments);
}

CutelystAction *Cutelyst::getAction(const QString &action, const QString &ns)
{
    Q_D(Cutelyst);
    return d->dispatcher->getAction(action, ns);
}

QList<CutelystAction *> Cutelyst::getActions(const QString &action, const QString &ns)
{
    Q_D(Cutelyst);
    return d->dispatcher->getActions(action, ns);
}

QList<CutelystPlugin::Plugin *> Cutelyst::plugins()
{
    Q_D(Cutelyst);
    return d->plugins.keys();
}

void Cutelyst::handleRequest(CutelystRequest *req, CutelystResponse *resp)
{
    Q_D(Cutelyst);

    d->request = req;
    d->response = resp;

    bool skipMethod = false;
    beforePrepareAction(this, &skipMethod);
    if (!skipMethod) {
        prepareAction();
        afterPrepareAction(this);

        beforeDispatch(this);
        dispatch();
        afterDispatch(this);
    }

    d->status = finalize();
}

void Cutelyst::prepareAction()
{
    Q_D(Cutelyst);
    d->dispatcher->prepareAction(this);
}

void Cutelyst::finalizeHeaders()
{
    Q_D(Cutelyst);

    CutelystResponse *response = d->response;
    if (response->finalizedHeaders()) {
        return;
    }

    if (response->location().isValid()) {
        response->addHeaderValue(QLatin1String("Location"), response->location().toEncoded());

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
            response->setContentType(QLatin1String("text/html; charset=utf-8"));
        }
    }

    if (response->hasBody()) {
        response->setContentLength(response->body().size());
    }

    finalizeCookies();

    d->engine->finalizeHeaders(this);
}

void Cutelyst::finalizeCookies()
{
    Q_D(Cutelyst);
    d->engine->finalizeCookies(this);
}

void Cutelyst::finalizeBody()
{
    Q_D(Cutelyst);
    d->engine->finalizeBody(this);
}

void Cutelyst::finalizeError()
{
    Q_D(Cutelyst);
    d->engine->finalizeError(this);
}

int Cutelyst::finalize()
{
    Q_D(Cutelyst);

    if (error()) {
        finalizeError();
    }

    finalizeHeaders();

    if (d->request->method() == "HEAD") {
        d->response->body().clear();
    }

    finalizeBody();

    if (d->stats) {
        qDebug("Request took: %d ms", d->stats->elapsed());
    }

    return d->response->status();
}

QVariant Cutelyst::pluginProperty(CutelystPlugin::Plugin * const plugin, const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Cutelyst);
    return d->plugins.value(plugin).value(key, defaultValue);
}

void Cutelyst::setPluginProperty(CutelystPlugin::Plugin *plugin, const QString &key, const QVariant &value)
{
    Q_D(Cutelyst);
    d->plugins[plugin].insert(key, value);
}

CutelystPrivate::CutelystPrivate() :
    action(0),
    detached(false),
    state(false),
    stats(0)
{
    stats = new QTime;
    stats->start();
}
