/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "enginerequest.h"

#include "config.h"

#include <QUrl>
#include <QUrlQuery>
#include <QCoreApplication>
#include <QBuffer>

using namespace Cutelyst;

Context::Context(ContextPrivate *priv) : d_ptr(priv)
{
}

Context::Context(Application *app) :
    d_ptr(new ContextPrivate(app, app->engine(), app->dispatcher(), app->plugins()))
{
    d_ptr->response = new Response(this, d_ptr->engine, app->defaultHeaders());

    DummyRequest req(this);
    req.body = new QBuffer(this);
    req.body->open(QBuffer::ReadWrite);

    d_ptr->request = new Request(new RequestPrivate(nullptr));
    d_ptr->request->setParent(this);
    d_ptr->request->d_ptr->engine = d_ptr->engine;
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
    if (error.isEmpty()) {
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

void Context::setStash(const QString &key, const ParamsMultiMap &map)
{
    Q_D(Context);
    d->stash.insert(key, QVariant::fromValue(map));
}

QStack<Component *> Context::stack() const
{
    Q_D(const Context);
    return d->stack;
}

QUrl Context::uriFor(const QString &path, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl uri = d->request->uri();

    QString _path = path;
    if (_path.isEmpty()) {
        // ns must NOT return a leading slash
        _path = QLatin1Char('/') + d->action->controller()->ns();
    } else if (!_path.startsWith(QLatin1Char('/'))) {
        _path.prepend(QLatin1Char('/'));
    }

    if (!args.isEmpty()) {
        _path = _path + QLatin1Char('/') + args.join(QLatin1Char('/'));
    }
    uri.setPath(_path, QUrl::DecodedMode);

    QUrlQuery query;
    if (!queryValues.isEmpty()) {
        // Avoid a trailing '?'
        if (queryValues.size()) {
            auto it = queryValues.constBegin();
            const auto end = queryValues.constEnd();
            while (it != end) {
                query.addQueryItem(it.key(), it.value());
                ++it;
            }
        }
    }
    uri.setQuery(query);

    return uri;
}

QUrl Context::uriFor(Action *action, const QStringList &captures, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl uri;
    Action *localAction = action;
    if (!localAction) {
        localAction = d->action;
    }

    QStringList localArgs = args;
    QStringList localCaptures = captures;

    Action *expandedAction = d->dispatcher->expandAction(this, action);
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
    if (path.isEmpty()) {
        qCWarning(CUTELYST_CORE) << "Can not find action for" << localAction << localCaptures;
        return uri;
    }

    uri = uriFor(path, localArgs, queryValues);
    return uri;
}

QUrl Context::uriForAction(const QString &path, const QStringList &captures, const QStringList &args, const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl uri;
    Action *action = d->dispatcher->getActionByPath(path);
    if (!action) {
        qCWarning(CUTELYST_CORE) << "Can not find action for" << path;
        return uri;
    }

    uri = uriFor(action, captures, args, queryValues);
    return uri;
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

Action *Context::getAction(const QString &action, const QString &ns) const
{
    Q_D(const Context);
    return d->dispatcher->getAction(action, ns);
}

QVector<Action *> Context::getActions(const QString &action, const QString &ns) const
{
    Q_D(const Context);
    return d->dispatcher->getActions(action, ns);
}

QVector<Cutelyst::Plugin *> Context::plugins() const
{
    Q_D(const Context);
    return d->plugins;
}

bool Context::execute(Component *code)
{
    Q_D(Context);
    Q_ASSERT_X(code, "Context::execute", "trying to execute a null Cutelyst::Component");

    static int recursion = qEnvironmentVariableIsSet("RECURSION") ? qEnvironmentVariableIntValue("RECURSION") : 1000;
    if (d->stack.size() >= recursion) {
        QString msg = QStringLiteral("Deep recursion detected (stack size %1) calling %2, %3")
                .arg(QString::number(d->stack.size()), code->reverse(), code->name());
        error(msg);
        setState(false);
        return false;
    }

    bool ret;
    d->stack.push(code);

    if (d->stats) {
        const QString statsInfo = d->statsStartExecute(code);

        ret = code->execute(this);

        if (!statsInfo.isEmpty()) {
            d->statsFinishExecute(statsInfo);
        }
    } else {
        ret = code->execute(this);
    }

    d->stack.pop();

    return ret;
}

QLocale Context::locale() const
{
    Q_D(const Context);
    return d->locale;
}

void Context::setLocale(const QLocale &locale)
{
    Q_D(Context);
    d->locale = locale;
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

QString Context::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    Q_D(const Context);
    return d->app->translate(d->locale, context, sourceText, disambiguation, n);
}

QString ContextPrivate::statsStartExecute(Component *code)
{
    QString actionName;
    // Skip internal actions
    if (code->name().startsWith(QLatin1Char('_'))) {
        return actionName;
    }

    actionName = code->reverse();

    if (qobject_cast<Action *>(code)) {
        actionName.prepend(QLatin1Char('/'));
    }

    if (stack.size() > 2) {
        actionName = QLatin1String("-> ") + actionName;
        actionName = actionName.rightJustified(actionName.size() + stack.size() - 2, QLatin1Char(' '));
    }

    stats->profileStart(actionName);

    return actionName;
}

void ContextPrivate::statsFinishExecute(const QString &statsInfo)
{
    stats->profileEnd(statsInfo);
}

#include "moc_context.cpp"
#include "moc_context_p.cpp"
