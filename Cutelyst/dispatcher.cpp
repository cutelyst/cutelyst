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
using namespace Qt::Literals::StringLiterals;

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

void Dispatcher::setupActions(const QVector<Controller *> &controllers,
                              const QVector<Cutelyst::DispatchType *> &dispatchers,
                              bool printActions)
{
    Q_D(Dispatcher);

    d->dispatchers = dispatchers;

    ActionList registeredActions;
    for (Controller *controllerItem : controllers) {
        bool instanceUsed  = false;
        const auto actions = controllerItem->actions();
        for (Action *action : actions) {
            bool registered = false;
            if (!d->actions.contains(action->reverse())) {
                if (!action->attributes().contains(u"Private"_s)) {
                    // Register the action with each dispatcher
                    for (DispatchType *dispatcher : dispatchers) {
                        if (dispatcher->registerAction(action)) {
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
                const QString name = action->ns() + u'/' + action->name();
                d->actions.insert(name, {name, action});
                auto it = d->actionContainer.find(action->ns());
                if (it != d->actionContainer.end()) {
                    it->actions << action;
                } else {
                    d->actionContainer.insert(action->ns(), {action->ns(), {action}});
                }

                registeredActions.append(action);
                instanceUsed = true;
            } else {
                qCDebug(CUTELYST_DISPATCHER)
                    << "The action" << action->name() << "of" << action->controller()->objectName()
                    << "controller was not registered in any dispatcher."
                       " If you still want to access it internally (via actionFor())"
                       " you may make it's method private.";
            }
        }

        if (instanceUsed) {
            d->controllers.insert(controllerItem->objectName(),
                                  {controllerItem->objectName(), controllerItem});
        }
    }

    if (printActions) {
        d->printActions();
    }

    // Cache root actions, BEFORE the controllers set them
    d->rootActions = d->actionContainer.value(u"").actions;

    for (const Controller *controllerItem : controllers) {
        controllerItem->d_ptr->setupFinished();
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
        for (const DispatchType *dispatcher : dispatchers) {
            qCDebug(CUTELYST_DISPATCHER) << dispatcher->list().constData();
        }
    }
}

bool Dispatcher::dispatch(Context *c)
{
    const Action *action = c->action();
    if (action) {
        return action->controller()->_DISPATCH(c);
    } else {
        const QString path = c->req()->path();
        if (path.isEmpty()) {
            //% "No default action defined."
            c->appendError(c->qtTrId("cutelyst-dispatcher-no-default-act"));
        } else {
            //% "Unknown resource '%1'."
            c->appendError(c->qtTrId("cutelyst-dispatcher-unknown-resource").arg(path));
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

bool Dispatcher::forward(Context *c, QStringView opname)
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

    const Request *request = c->request();
    d->prepareAction(c, request->path());

    static const bool log = CUTELYST_DISPATCHER().isDebugEnabled();
    if (log) {
        if (!request->match().isEmpty()) {
            qCDebug(CUTELYST_DISPATCHER) << "Path is" << request->match();
        }

        if (!request->args().isEmpty()) {
            qCDebug(CUTELYST_DISPATCHER) << "Arguments are" << request->args().join(u'/');
        }
    }
}

void DispatcherPrivate::prepareAction(Context *c, QStringView path) const
{
    QStringList args;

    //  "/foo/bar"
    //  "/foo/" skip
    //  "/foo"
    //  "/"
    Q_FOREVER
    {
        // Check out the dispatch types to see if any
        // will handle the path at this level
        bool matched = std::ranges::any_of(dispatchers, [&](const DispatchType *type) {
            return type->match(c, path, args) == DispatchType::ExactMatch;
        });
        if (matched) {
            return;
        }

        // leave the loop if we are at the root "/"
        if (path.length() == 1) {
            break;
        }

        int pos = path.lastIndexOf(u'/');

        args.emplaceFront(path.mid(pos + 1).toString());

        if (pos == 0) {
            path.truncate(pos + 1);
        } else {
            path.truncate(pos);
        }
    }
}

Action *Dispatcher::getAction(QStringView name, QStringView nameSpace) const
{
    Q_D(const Dispatcher);

    if (name.isEmpty()) {
        return nullptr;
    }

    if (nameSpace.isEmpty()) {
        const QString normName = u'/' + name;
        return d->actions.value(normName).action;
    }

    return getActionByPath(QString{nameSpace + u'/' + name});
}

Action *Dispatcher::getActionByPath(QStringView path) const
{
    Q_D(const Dispatcher);

    int slashes = path.count(u'/');
    if (slashes == 0) {
        return d->actions.value(QString{u'/' + path}).action;
    } else if (path.startsWith(u'/') && slashes != 1) {
        return d->actions.value(path.mid(1)).action;
    }
    return d->actions.value(path).action;
}

ActionList Dispatcher::getActions(QStringView name, QStringView nameSpace) const
{
    Q_D(const Dispatcher);

    ActionList ret;

    if (name.isEmpty()) {
        return ret;
    }

    const ActionList containers = d->getContainers(nameSpace);
    auto rIt                    = containers.rbegin();
    while (rIt != containers.rend()) {
        if ((*rIt)->name() == name) {
            ret.append(*rIt);
        }
        ++rIt;
    }
    return ret;
}

Controller *Dispatcher::controller(QStringView name) const
{
    Q_D(const Dispatcher);
    return d->controllers.value(name).controller;
}

QList<Controller *> Dispatcher::controllers() const
{
    Q_D(const Dispatcher);
    QList<Controller *> ret;
    for (const auto &value : d->controllers) {
        ret.append(value.controller);
    }
    return ret;
}

QString Dispatcher::uriForAction(Action *action, const QStringList &captures) const
{
    Q_D(const Dispatcher);
    QString ret;
    if (Q_UNLIKELY(action == nullptr)) {
        qCCritical(CUTELYST_DISPATCHER) << "Dispatcher::uriForAction called with null action";
        ret = u"/"_s;
    } else {
        for (const DispatchType *dispatcher : d->dispatchers) {
            ret = dispatcher->uriForAction(action, captures);
            if (!ret.isNull()) {
                if (ret.isEmpty()) { // cppcheck-suppress knownConditionTrueFalse
                    ret = u"/"_s;
                }
                break;
            }
        }
    }
    return ret;
}

Action *Dispatcher::expandAction(const Context *c, Action *action) const
{
    Q_D(const Dispatcher);
    for (const DispatchType *dispatcher : d->dispatchers) {
        Action *expandedAction = dispatcher->expandAction(c, action);
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

void DispatcherPrivate::printActions() const
{
    QVector<QStringList> table;

    auto keys = actions.keys();
    std::ranges::sort(keys);
    for (const auto &key : std::as_const(keys)) {
        const Action *action = actions.value(key).action;
        QString path         = key.toString();
        if (!path.startsWith(u'/')) {
            path.prepend(u'/');
        }

        QStringList row;
        row.append(path);
        row.append(action->className());
        row.append(action->name());
        table.append(row);
    }

    qCDebug(CUTELYST_DISPATCHER) << Utils::buildTable(table,
                                                      {
                                                          u"Private"_s,
                                                          u"Class"_s,
                                                          u"Method"_s,
                                                      },
                                                      u"Loaded Private actions:"_s)
                                        .constData();
}

ActionList DispatcherPrivate::getContainers(QStringView ns) const
{
    ActionList ret;

    if (ns.compare(u"/") != 0) {
        int pos = ns.size();
        //        qDebug() << pos << ns.mid(0, pos);
        while (pos > 0) {
            //            qDebug() << pos << ns.mid(0, pos);
            ret.append(actionContainer.value(ns.mid(0, pos)).actions);
            pos = ns.lastIndexOf(u'/', pos - 1);
        }
    }
    //    qDebug() << actionContainer.size() << rootActions;
    ret.append(rootActions);

    return ret;
}

Action *DispatcherPrivate::command2Action(const Context *c,
                                          QStringView command,
                                          const QStringList &args) const
{
    auto it = actions.constFind(command);
    if (it != actions.constEnd()) {
        return it.value().action;
    }

    return invokeAsPath(c, command, args);
}

Action *DispatcherPrivate::invokeAsPath(const Context *c,
                                        QStringView relativePath,
                                        const QStringList &args) const
{
    Q_UNUSED(args);
    Q_Q(const Dispatcher);

    Action *ret;
    const QString path = DispatcherPrivate::actionRel2Abs(c, relativePath);
    QStringView pathView{path};

    int pos     = pathView.lastIndexOf(u'/');
    int lastPos = pathView.size();
    do {
        if (pos == -1) {
            ret = q->getAction(pathView);
            if (ret) {
                return ret;
            }
        } else {
            const auto name = pathView.mid(pos + 1, lastPos);
            pathView        = pathView.mid(0, pos);
            ret             = q->getAction(name, pathView);
            if (ret) {
                return ret;
            }
        }

        lastPos = pos;
        pos     = pathView.indexOf(u'/', pos - 1);
    } while (pos != -1);

    return nullptr;
}

QString DispatcherPrivate::actionRel2Abs(const Context *c, QStringView path)
{
    QString ret;
    if (path.startsWith(u'/')) {
        ret = path.mid(1).toString();
        return ret;
    }

    const QString ns = qobject_cast<Action *>(c->stack().constLast())->ns();
    if (ns.isEmpty()) {
        ret = path.toString();
    } else {
        ret = ns + u'/' + path;
    }
    return ret;
}

#include "moc_dispatcher.cpp"
