/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "action.h"
#include "application.h"
#include "common.h"
#include "context.h"
#include "controller.h"
#include "controller_p.h"
#include "dispatcher_p.h"
#include "dispatchtypechained.h"
#include "dispatchtypepath.h"
#include "engine.h"
#include "request_p.h"
#include "utils.h"

#include <QMetaMethod>
#include <QUrl>

using namespace Cutelyst;

Dispatcher::Dispatcher(QObject *parent)
    : QObject(parent)
    , d_ptr(new DispatcherPrivate(this))
{
    new DispatchTypePath(parent);
    new DispatchTypeChained(parent);
}

Dispatcher::~Dispatcher()
{
    delete d_ptr;
}

void Dispatcher::setupActions(const QVector<Controller *> &controllers, const QVector<Cutelyst::DispatchType *> &dispatchers, bool printActions)
{
    Q_D(Dispatcher);

    d->dispatchers = dispatchers;

    ActionList registeredActions;
    for (Controller *controller : controllers) {
        bool instanceUsed  = false;
        const auto actions = controller->actions();
        for (Action *action : actions) {
            bool registered = false;
            if (!d->actions.contains(action->reverse())) {
                if (!action->attributes().contains(QStringLiteral("Private"))) {
                    // Register the action with each dispatcher
                    for (DispatchType *dispatch : dispatchers) {
                        if (dispatch->registerAction(action)) {
                            registered = true;
                        }
                    }
                } else {
                    // We register private actions
                    registered = true;
                }
            }

            // The Begin, Auto, End actions are not
            // registered by Dispatchers but we need them
            // as private actions anyway
            if (registered) {
                const QString name = action->ns() + QLatin1Char('/') + action->name();
                d->actions.insert(name, {name, action});
                d->actionContainer[action->ns()] << action;
                registeredActions.append(action);
                instanceUsed = true;
            } else {
                qCDebug(CUTELYST_DISPATCHER) << "The action" << action->name() << "of"
                                             << action->controller()->objectName()
                                             << "controller was not registered in any dispatcher."
                                                " If you still want to access it internally (via actionFor())"
                                                " you may make it's method private.";
            }
        }

        if (instanceUsed) {
            d->controllers.insert(controller->objectName(), controller);
        }
    }

    if (printActions) {
        d->printActions();
    }

    // Cache root actions, BEFORE the controllers set them
    d->rootActions = d->actionContainer.value(QLatin1String(""));

    for (Controller *controller : controllers) {
        controller->d_ptr->setupFinished();
    }

    // Unregister any dispatcher that is not in use
    int i = 0;
    while (i < d->dispatchers.size()) {
        DispatchType *type = d->dispatchers.at(i);
        if (!type->inUse()) {
            d->dispatchers.removeAt(i);
            continue;
        }
        ++i;
    }

    if (printActions) {
        // List all public actions
        for (DispatchType *dispatch : dispatchers) {
            qCDebug(CUTELYST_DISPATCHER) << dispatch->list().constData();
        }
    }
}

bool Dispatcher::dispatch(Context *c)
{
    Action *action = c->action();
    if (action) {
        return action->controller()->_DISPATCH(c);
    } else {
        const QString path = c->req()->path();
        if (path.isEmpty()) {
            c->error(c->translate("Cutelyst::Dispatcher", "No default action defined"));
        } else {
            c->error(c->translate("Cutelyst::Dispatcher", "Unknown resource '%1'.").arg(path));
        }
    }
    return false;
}

bool Dispatcher::forward(Context *c, Component *component)
{
    Q_ASSERT(component);
    // If the component was an Action
    // the dispatch() would call c->execute
    return c->execute(component);
}

bool Dispatcher::forward(Context *c, const QString &opname)
{
    Q_D(const Dispatcher);

    Action *action = d->command2Action(c, opname, c->request()->args());
    if (action) {
        return action->dispatch(c);
    }

    qCCritical(CUTELYST_DISPATCHER) << "Action not found" << opname << c->request()->args();
    return false;
}

void Dispatcher::prepareAction(Context *c)
{
    Q_D(Dispatcher);

    Request *request = c->request();
    d->prepareAction(c, request->path());

    static const auto &log = CUTELYST_DISPATCHER();
    if (log.isDebugEnabled()) {
        if (!request->match().isEmpty()) {
            qCDebug(log) << "Path is" << request->match();
        }

        if (!request->args().isEmpty()) {
            qCDebug(log) << "Arguments are" << request->args().join(QLatin1Char('/'));
        }
    }
}

void DispatcherPrivate::prepareAction(Context *c, const QString &requestPath) const
{
    QString path = normalizePath(requestPath);
    QStringList args;

    //  "foo/bar"
    //  "foo/" skip
    //  "foo"
    //  ""
    Q_FOREVER
    {
        // Check out the dispatch types to see if any
        // will handle the path at this level
        for (DispatchType *type : dispatchers) {
            if (type->match(c, path, args) == DispatchType::ExactMatch) {
                return;
            }
        }

        // leave the loop if we are at the root "/"
        if (path.isEmpty()) {
            break;
        }

        int pos = path.lastIndexOf(u'/');

        const QString arg = path.mid(pos + 1);
        args.prepend(arg);

        path.resize(pos);
    }
}

Action *Dispatcher::getAction(const QString &name, const QString &nameSpace) const
{
    Q_D(const Dispatcher);

    if (name.isEmpty()) {
        return nullptr;
    }

    if (nameSpace.isEmpty()) {
        const QString normName = u'/' + name;
        return d->actions.value(normName).action;
    }

    const QString ns = DispatcherPrivate::cleanNamespace(nameSpace);
    return getActionByPath(ns + u'/' + name);
}

Action *Dispatcher::getActionByPath(const QString &path) const
{
    Q_D(const Dispatcher);

    QString _path = path;
    int slashes   = _path.count(u'/');
    if (slashes == 0) {
        _path.prepend(u'/');
    } else if (_path.startsWith(u'/') && slashes != 1) {
        _path.remove(0, 1);
    }
    return d->actions.value(_path).action;
}

ActionList Dispatcher::getActions(const QString &name, const QString &nameSpace) const
{
    Q_D(const Dispatcher);

    ActionList ret;

    if (name.isEmpty()) {
        return ret;
    }

    const QString ns            = DispatcherPrivate::cleanNamespace(nameSpace);
    const ActionList containers = d->getContainers(ns);
    auto rIt                    = containers.rbegin();
    while (rIt != containers.rend()) {
        if ((*rIt)->name() == name) {
            ret.append(*rIt);
        }
        ++rIt;
    }
    return ret;
}

QMap<QString, Controller *> Dispatcher::controllers() const
{
    Q_D(const Dispatcher);
    return d->controllers;
}

QString Dispatcher::uriForAction(Action *action, const QStringList &captures) const
{
    Q_D(const Dispatcher);
    QString ret;
    for (DispatchType *dispatch : d->dispatchers) {
        ret = dispatch->uriForAction(action, captures);
        if (!ret.isNull()) {
            if (ret.isEmpty()) {
                ret = QStringLiteral("/");
            }
            break;
        }
    }
    return ret;
}

Action *Dispatcher::expandAction(const Context *c, Action *action) const
{
    Q_D(const Dispatcher);
    for (DispatchType *dispatch : d->dispatchers) {
        Action *expandedAction = dispatch->expandAction(c, action);
        if (expandedAction) {
            return expandedAction;
        }
    }
    return action;
}

QVector<DispatchType *> Dispatcher::dispatchers() const
{
    Q_D(const Dispatcher);
    return d->dispatchers;
}

QString DispatcherPrivate::cleanNamespace(const QString &ns)
{
    QString ret       = ns;
    bool lastWasSlash = true; // remove initial slash
    int nsSize        = ns.size();
    for (int i = 0; i < nsSize; ++i) {
        // Mark if the last char was a slash
        // so that two or more consecutive slashes
        // could be converted to just one
        // "a///b" -> "a/b"
        if (ret.at(i) == u'/') {
            if (lastWasSlash) {
                ret.remove(i, 1);
                --nsSize;
            } else {
                lastWasSlash = true;
            }
        } else {
            lastWasSlash = false;
        }
    }
    return ret;
}

QString DispatcherPrivate::normalizePath(const QString &path)
{
    QString ret    = path;
    bool lastSlash = true;
    int i          = 0;
    while (i < ret.size()) {
        if (ret.at(i) == u'/') {
            if (lastSlash) {
                ret.remove(i, 1);
                continue;
            }
            lastSlash = true;
        } else {
            lastSlash = false;
        }
        ++i;
    }

    if (ret.endsWith(u'/')) {
        ret.resize(ret.size() - 1);
    }
    return ret;
}

void DispatcherPrivate::printActions() const
{
    QVector<QStringList> table;

    auto keys = actions.keys();
    std::sort(keys.begin(), keys.end());
    for (const auto &key : keys) {
        Action *action = actions.value(key).action;
        QString path   = key.toString();
        if (!path.startsWith(u'/')) {
            path.prepend(u'/');
        }

        QStringList row;
        row.append(path);
        row.append(action->className());
        row.append(action->name());
        table.append(row);
    }

    qCDebug(CUTELYST_DISPATCHER) << Utils::buildTable(table, {QLatin1String("Private"), QLatin1String("Class"), QLatin1String("Method")}, QLatin1String("Loaded Private actions:")).constData();
}

ActionList DispatcherPrivate::getContainers(const QString &ns) const
{
    ActionList ret;

    if (ns.compare(u"/") != 0) {
        int pos = ns.size();
        //        qDebug() << pos << ns.mid(0, pos);
        while (pos > 0) {
            //            qDebug() << pos << ns.mid(0, pos);
            ret.append(actionContainer.value(ns.mid(0, pos)));
            pos = ns.lastIndexOf(QLatin1Char('/'), pos - 1);
        }
    }
    //    qDebug() << actionContainer.size() << rootActions;
    ret.append(rootActions);

    return ret;
}

Action *DispatcherPrivate::command2Action(Context *c, const QString &command, const QStringList &args) const
{
    auto it = actions.constFind(command);
    if (it != actions.constEnd()) {
        return it.value().action;
    }

    return invokeAsPath(c, command, args);
}

Action *DispatcherPrivate::invokeAsPath(Context *c, const QString &relativePath, const QStringList &args) const
{
    Q_UNUSED(args);
    Q_Q(const Dispatcher);

    Action *ret;
    QString path = DispatcherPrivate::actionRel2Abs(c, relativePath);

    int pos     = path.lastIndexOf(QLatin1Char('/'));
    int lastPos = path.size();
    do {
        if (pos == -1) {
            ret = q->getAction(path, QString());
            if (ret) {
                return ret;
            }
        } else {
            const QString name = path.mid(pos + 1, lastPos);
            path               = path.mid(0, pos);
            ret                = q->getAction(name, path);
            if (ret) {
                return ret;
            }
        }

        lastPos = pos;
        pos     = path.indexOf(QLatin1Char('/'), pos - 1);
    } while (pos != -1);

    return nullptr;
}

QString DispatcherPrivate::actionRel2Abs(Context *c, const QString &path)
{
    QString ret;
    if (path.startsWith(QLatin1Char('/'))) {
        ret = path.mid(1);
        return ret;
    }

    const QString ns = qobject_cast<Action *>(c->stack().constLast())->ns();
    if (ns.isEmpty()) {
        ret = path;
    } else {
        ret = ns + QLatin1Char('/') + path;
    }
    return ret;
}

#include "moc_dispatcher.cpp"
