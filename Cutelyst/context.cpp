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

#include "context_p.h"

#include "common.h"
#include "request.h"
#include "response.h"
#include "action.h"
#include "dispatcher.h"
#include "controller.h"
#include "application.h"
#include "stats.h"

#include "config.h"

#include <QUrl>
#include <QUrlQuery>
#include <QStringBuilder>
#include <QCoreApplication>

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
    if (error.isNull()) {
        d->error.clear();
    } else {
        d->error << error;
        qCCritical(CUTELYST_CORE) << error;
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
    return QString::fromLatin1(d->action->controller()->metaObject()->className());
}

Controller *Context::controller() const
{
    Q_D(const Context);
    return d->action->controller();
}

Controller *Context::controller(const QString &name) const
{
    Q_D(const Context);
    return d->dispatcher->controllers().value(name);
}

View *Context::view() const
{
    Q_D(const Context);
    return d->view;
}

View *Context::view(const QString &name) const
{
    Q_D(const Context);
    return d->app->view(name);
}

bool Context::setView(const QString &name)
{
    Q_D(Context);
    d->view = d->app->view(name);
    return d->view;
}

QVariantHash &Context::stash()
{
    Q_D(Context);
    return d->stash;
}

QVariant Context::stash(const QString &key) const
{
    Q_D(const Context);
    return d->stash.value(key);
}

void Context::setStash(const QString &key, const QVariant &value)
{
    Q_D(Context);
    d->stash.insert(key, value);
}

QStack<Component *> Context::stack() const
{
    Q_D(const Context);
    return d->stack;
}

QUrl Context::uriFor(const QString &path, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl ret = d->request->uri();

    QString _path = path;
    if (_path.isEmpty()) {
        _path = d->action->controller()->ns();
    }

    if (!args.isEmpty()) {
        QStringList encodedArgs;
        encodedArgs.append(_path);

        Q_FOREACH (const QString &arg, args) {
            encodedArgs.append(QString::fromLatin1(QUrl::toPercentEncoding(arg)));
        }
        ret.setPath(QLatin1Char('/') % encodedArgs.join(QLatin1Char('/')));
    } else if (_path.startsWith(QLatin1Char('/'))) {
        ret.setPath(_path);
    } else {
        ret.setPath(QLatin1Char('/') % _path);
    }

    if (!queryValues.isEmpty()) {
        // Avoid a trailing '?'
        QUrlQuery query;
        if (queryValues.size()) {
            auto it = queryValues.constBegin();
            while (it != queryValues.constEnd()) {
                query.addQueryItem(it.key(), it.value());
                ++it;
            }
        }
        ret.setQuery(query);
    }

    return ret;
}

QUrl Context::uriFor(Action *action, const QStringList &captures, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    Action *localAction = action;
    if (!localAction) {
        localAction = d->action;
    }

    QStringList localArgs = args;
    QStringList localCaptures = captures;

    Action *expandedAction = d->dispatcher->expandAction(const_cast<Context*>(this), action);
    if (expandedAction->numberOfCaptures() > 0) {
        while (localCaptures.size() < expandedAction->numberOfCaptures()
               && localArgs.size()) {
            localCaptures.append(localArgs.takeFirst());
        }
    } else {
        QStringList localCapturesAux = localCaptures;
        localCapturesAux.append(localArgs);
        localArgs = localCapturesAux;
        localCaptures = QStringList();
    }

    const QString path = d->dispatcher->uriForAction(localAction, localCaptures);

    return uriFor(path, localArgs, queryValues);
}

QUrl Context::uriForAction(const QString &path, const QStringList &captures, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    Action *action = d->dispatcher->getActionByPath(path);
    if (!action) {
        qCWarning(CUTELYST_CORE) << "Can not find action for" << path;
        return QUrl();
    }
    return uriFor(action, captures, args, queryValues);
}

bool Context::detached() const
{
    Q_D(const Context);
    return d->detached;
}

void Context::detach(Action *action)
{
    Q_D(Context);
    if (action) {
        d->dispatcher->forward(this, action);
    }
    d->detached = true;
}

bool Context::forward(Component *action)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action);
}

bool Context::forward(const QString &action)
{
    Q_D(Context);
    return d->dispatcher->forward(this, action);
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

QList<Cutelyst::Plugin *> Context::plugins()
{
    Q_D(Context);
    return d->plugins;
}

bool Context::execute(Component *code)
{
    Q_D(Context);
    Q_ASSERT_X(code, "Context::execute", "trying to execute a null Cutelyst::Component");

    bool ret;
    d->stack.push(code);

    if (d->stats) {
        QString statsInfo = d->statsStartExecute(code);

        ret = code->execute(this);

        if (!statsInfo.isNull()) {
            d->statsFinishExecute(statsInfo);
        }
    } else {
        ret = code->execute(this);
    }

    d->stack.pop();

    return ret;
}

QVariant Context::config(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Context);
    return d->app->config(key, defaultValue);
}

QVariantMap Context::config() const
{
    Q_D(const Context);
    return d->app->config();
}

QByteArray Context::welcomeMessage() const
{
    const QString name = QCoreApplication::applicationName();
    response()->setContentType(QLatin1String("text/html; charset=utf-8"));
    QByteArray ret;
    QTextStream out(&ret);
    out << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
           "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
           "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
           "    <head>\n"
           "        <meta http-equiv=\"Content-Language\" content=\"en\" />\n"
           "        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
           "        <title>" << name << " on Cutelyst " << VERSION << "</title>\n"
           "    </head>\n"
           "    <body>\n"
           "        <div id=\"content\">\n"
           "            <div id=\"topbar\">\n"
           "               <h1><span id=\"appname\">" << name << "</span> on <a href=\"http://cutelyst.org\">Cutelyst</a>\n"
           "                   " << VERSION << "</h1>\n"
           "             </div>\n"
           "         </div>\n"
           "    <body>\n"
           "</html>\n";
    return ret;
}

void *Context::engineData()
{
    Q_D(const Context);
    return d->requestPtr;
}

QString ContextPrivate::statsStartExecute(Component *code)
{
    // Skip internal actions
    if (code->name().startsWith(QLatin1Char('_'))) {
        return QString();
    }

    QString actionName = code->reverse();

    if (dynamic_cast<Action *>(code)) {
        actionName = QLatin1Char('/') % actionName;
    }

    if (stack.size() > 2) {
        actionName = QLatin1String("-> ") % actionName;
        actionName = actionName.rightJustified(actionName.size() + stack.size() - 2, QLatin1Char(' '));
    }

    stats->profileStart(actionName);

    return actionName;
}


void ContextPrivate::statsFinishExecute(const QString &statsInfo)
{
    stats->profileEnd(statsInfo);
}
