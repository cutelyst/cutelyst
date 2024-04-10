/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "action.h"
#include "application.h"
#include "common.h"
#include "config.h"
#include "context_p.h"
#include "controller.h"
#include "dispatcher.h"
#include "enginerequest.h"
#include "request.h"
#include "response.h"
#include "stats.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QUrl>
#include <QUrlQuery>

using namespace Cutelyst;

Context::Context(ContextPrivate *priv)
    : d_ptr(priv)
{
}

Context::Context(Application *app)
    : d_ptr(new ContextPrivate(app, app->engine(), app->dispatcher(), app->plugins()))
{
    auto req  = new DummyRequest(this);
    req->body = new QBuffer(this);
    req->body->open(QBuffer::ReadWrite);
    req->context = this;

    d_ptr->response               = new Response(app->defaultHeaders(), req);
    d_ptr->request                = new Request(req);
    d_ptr->request->d_ptr->engine = d_ptr->engine;
}

Context::~Context()
{
    delete d_ptr->request;
    delete d_ptr->response;
    delete d_ptr;
}

bool Context::error() const noexcept
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

QStringList Context::errors() const noexcept
{
    Q_D(const Context);
    return d->error;
}

bool Context::state() const noexcept
{
    Q_D(const Context);
    return d->state;
}

void Context::setState(bool state) noexcept
{
    Q_D(Context);
    d->state = state;
}

Engine *Context::engine() const noexcept
{
    Q_D(const Context);
    return d->engine;
}

Application *Context::app() const noexcept
{
    Q_D(const Context);
    return d->app;
}

Response *Context::response() const noexcept
{
    Q_D(const Context);
    return d->response;
}

Response *Context::res() const noexcept
{
    Q_D(const Context);
    return d->response;
}

Action *Context::action() const noexcept
{
    Q_D(const Context);
    return d->action;
}

QString Context::actionName() const noexcept
{
    Q_D(const Context);
    return d->action->name();
}

QString Context::ns() const noexcept
{
    Q_D(const Context);
    return d->action->ns();
}

Request *Context::request() const noexcept
{
    Q_D(const Context);
    return d->request;
}

Request *Context::req() const noexcept
{
    Q_D(const Context);
    return d->request;
}

Dispatcher *Context::dispatcher() const noexcept
{
    Q_D(const Context);
    return d->dispatcher;
}

QString Cutelyst::Context::controllerName() const
{
    Q_D(const Context);
    return QString::fromLatin1(d->action->controller()->metaObject()->className());
}

Controller *Context::controller() const noexcept
{
    Q_D(const Context);
    return d->action->controller();
}

Controller *Context::controller(const QString &name) const
{
    Q_D(const Context);
    return d->dispatcher->controllers().value(name);
}

View *Context::customView() const noexcept
{
    Q_D(const Context);
    return d->view;
}

View *Context::view(const QString &name) const
{
    Q_D(const Context);
    return d->app->view(name);
}

View *Context::view(QStringView name) const
{
    Q_D(const Context);
    return d->app->view(name);
}

bool Context::setCustomView(const QString &name)
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

QVariant Context::stash(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Context);
    return d->stash.value(key, defaultValue);
}

QVariant Context::stashTake(const QString &key)
{
    Q_D(Context);
    return d->stash.take(key);
}

bool Context::stashRemove(const QString &key)
{
    Q_D(Context);
    return d->stash.remove(key);
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

QStack<Component *> Context::stack() const noexcept
{
    Q_D(const Context);
    return d->stack;
}

QUrl Context::uriFor(const QString &path,
                     const QStringList &args,
                     const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl uri = d->request->uri();

    QString _path;
    if (path.isEmpty()) {
        // ns must NOT return a leading slash
        const QString controllerNS = d->action->controller()->ns();
        if (!controllerNS.isEmpty()) {
            _path.prepend(controllerNS);
        }
    } else {
        _path = path;
    }

    if (!args.isEmpty()) {
        if (_path.compare(u"/") == 0) {
            _path += args.join(u'/');
        } else {
            _path = _path + u'/' + args.join(u'/');
        }
    }

    if (!_path.startsWith(u'/')) {
        _path.prepend(u'/');
    }
    uri.setPath(_path, QUrl::DecodedMode);

    QUrlQuery query;
    if (!queryValues.isEmpty()) {
        // Avoid a trailing '?'
        if (queryValues.size()) {
            auto it = queryValues.constEnd();
            while (it != queryValues.constBegin()) {
                --it;
                query.addQueryItem(it.key(), it.value());
            }
        }
    }
    uri.setQuery(query);

    return uri;
}

QUrl Context::uriFor(Action *action,
                     const QStringList &captures,
                     const QStringList &args,
                     const ParamsMultiMap &queryValues) const
{
    Q_D(const Context);

    QUrl uri;
    Action *localAction = action;
    if (!localAction) {
        localAction = d->action;
    }

    QStringList localArgs     = args;
    QStringList localCaptures = captures;

    Action *expandedAction = d->dispatcher->expandAction(this, action);
    if (expandedAction->numberOfCaptures() > 0) {
        while (localCaptures.size() < expandedAction->numberOfCaptures() && localArgs.size()) {
            localCaptures.append(localArgs.takeFirst());
        }
    } else {
        QStringList localCapturesAux = localCaptures;
        localCapturesAux.append(localArgs);
        localArgs     = localCapturesAux;
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

QUrl Context::uriForAction(const QString &path,
                           const QStringList &captures,
                           const QStringList &args,
                           const ParamsMultiMap &queryValues) const
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

bool Context::detached() const noexcept
{
    Q_D(const Context);
    return d->detached;
}

void Context::detach(Action *action)
{
    Q_D(Context);
    if (action) {
        d->dispatcher->forward(this, action);
    } else {
        d->detached = true;
    }
}

void Context::detachAsync() noexcept
{
    Q_D(Context);
    ++d->actionRefCount;
}

void Context::attachAsync()
{
    Q_D(Context);

    // ASync might be destroyed at the same stack level it was created
    // resulting in this method being called while it was caller,
    // allowing this method to call finished() twice, with
    // a null context the second time so we check also check
    // if the action stack is not empty to skip this method
    if (--d->actionRefCount || !d->stack.isEmpty()) {
        return;
    }

    if (Q_UNLIKELY(d->engineRequest->status & EngineRequest::Finalized)) {
        qCWarning(CUTELYST_ASYNC) << "Trying to async attach to a finalized request! Skipping...";
        return;
    }

    if (d->engineRequest->status & EngineRequest::Async) {
        while (!d->pendingAsync.isEmpty()) {
            Component *action = d->pendingAsync.dequeue();
            const bool ret    = execute(action);

            if (d->actionRefCount) {
                return;
            }

            if (!ret) {
                break; // we are finished
            }
        }

        Q_EMIT d->app->afterDispatch(this);

        finalize();
    }
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

    static int recursion =
        qEnvironmentVariableIsSet("RECURSION") ? qEnvironmentVariableIntValue("RECURSION") : 1000;
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

        // The request might finalize execution before returning
        // so it's wise to check for d->stats again
        if (d->stats && !statsInfo.isEmpty()) {
            d->statsFinishExecute(statsInfo);
        }
    } else {
        ret = code->execute(this);
    }

    d->stack.pop();

    return ret;
}

QLocale Context::locale() const noexcept
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

QVariantMap Context::config() const noexcept
{
    Q_D(const Context);
    return d->app->config();
}

QString Context::translate(const char *context,
                           const char *sourceText,
                           const char *disambiguation,
                           int n) const
{
    Q_D(const Context);
    return d->app->translate(d->locale, context, sourceText, disambiguation, n);
}

void Context::finalize()
{
    Q_D(Context);

    if (Q_UNLIKELY(d->engineRequest->status & EngineRequest::Finalized)) {
        qCWarning(CUTELYST_CORE) << "Trying to finalize a finalized request! Skipping...";
        return;
    }

    if (d->stats) {
        qCDebug(CUTELYST_STATS,
                "Response Code: %d; Content-Type: %s; Content-Length: %s",
                d->response->status(),
                qPrintable(d->response->headers().header(QStringLiteral("CONTENT_TYPE"),
                                                         QStringLiteral("unknown"))),
                qPrintable(d->response->headers().header(QStringLiteral("CONTENT_LENGTH"),
                                                         QStringLiteral("unknown"))));

        const double enlapsed = d->engineRequest->elapsed.nsecsElapsed() / 1000000000.0;
        QString average;
        if (enlapsed == 0.0) {
            average = QStringLiteral("??");
        } else {
            average = QString::number(1.0 / enlapsed, 'f');
            average.truncate(average.size() - 3);
        }
        qCInfo(CUTELYST_STATS) << qPrintable(QStringLiteral("Request took: %1s (%2/s)\n%3")
                                                 .arg(QString::number(enlapsed, 'f'),
                                                      average,
                                                      QString::fromLatin1(d->stats->report())));
        delete d->stats;
        d->stats = nullptr;
    }

    d->engineRequest->finalize();
}

QString ContextPrivate::statsStartExecute(Component *code)
{
    QString actionName;
    // Skip internal actions
    if (code->name().startsWith(u'_')) {
        return actionName;
    }

    actionName = code->reverse();

    if (qobject_cast<Action *>(code)) {
        actionName.prepend(u'/');
    }

    if (stack.size() > 2) {
        actionName = u"-> " + actionName;
        actionName =
            actionName.rightJustified(actionName.size() + stack.size() - 2, QLatin1Char(' '));
    }

    stats->profileStart(actionName);

    return actionName;
}

void ContextPrivate::statsFinishExecute(const QString &statsInfo)
{
    stats->profileEnd(statsInfo);
}

void Context::stash(const QVariantHash &unite)
{
    Q_D(Context);
    auto it = unite.constBegin();
    while (it != unite.constEnd()) {
        d->stash.insert(it.key(), it.value());
        ++it;
    }
}

#include "moc_context.cpp"
#include "moc_context_p.cpp"
