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

#include "dispatcher_p.h"

#include "common.h"
#include "context.h"
#include "controller.h"
#include "action.h"
#include "request_p.h"
#include "dispatchtypepath.h"

#include <QUrl>
#include <QMetaMethod>
#include <QStringBuilder>
#include <QDebug>

using namespace Cutelyst;

Dispatcher::Dispatcher(QObject *parent) :
    QObject(parent),
    d_ptr(new DispatcherPrivate)
{
}

Dispatcher::~Dispatcher()
{
    delete d_ptr;
}

void Dispatcher::setupActions(const QList<Controller*> &controllers)
{
    Q_D(Dispatcher);

    if (d->dispatchers.isEmpty()) {
        registerDispatchType(new DispatchTypePath(this));
    }

    Q_FOREACH (Controller *controller, controllers) {
        // App controller
//        qDebug() << "Found a controller:" << controller << meta->className();
        const QMetaObject *meta = controller->metaObject();
        controller->setObjectName(meta->className());
        bool instanceUsed = false;
        for (int i = 0; i < meta->methodCount(); ++i) {
            QMetaMethod method = meta->method(i);
            if (method.methodType() == QMetaMethod::Method ||
                    method.methodType() == QMetaMethod::Slot ||
                    method.parameterCount()) {
                bool registered = false;
                Action *action = new Action(method, controller);
                if (action->isValid() && !d->actionHash.contains(action->privateName())) {
                    if (!action->attributes().contains("Private")) {
                        // Register the action with each dispatcher
                        Q_FOREACH (DispatchType *dispatch, d->dispatchers) {
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
                if (registered || action->isValid()) {
                    d->actionHash.insert(action->privateName(), action);
                    d->containerHash[action->ns()] << action;
                    instanceUsed = true;
                } else {
                    delete action;
                }
            }
        }

        if (instanceUsed) {
            d->constrollerHash.insert(meta->className(), controller);
        }
    }

    Q_FOREACH (Controller *controller, controllers) {
        controller->setupActions(this);
    }

    qCDebug(CUTELYST_DISPATCHER) << endl << printActions().data() << endl;
}

bool Dispatcher::dispatch(Context *ctx)
{
    if (ctx->action()) {
        QByteArray action = ctx->ns();
        action.reserve(action.size() + 12);
        action.prepend('/');
        action.append("/_DISPATCH", 10);
        return forward(ctx, action);
    } else {
        QString error;
        const QString &path = ctx->req()->path();
        if (path.isEmpty()) {
            error = QLatin1String("No default action defined");
        } else {
            error = QLatin1String("Unknown resource \"") % path % QLatin1Char('"');
        }
        qCDebug(CUTELYST_DISPATCHER) << error;
        ctx->error(error);
    }
    return false;
}

bool Dispatcher::forward(Context *ctx, const QByteArray &opname, const QStringList &arguments)
{
    Action *action = command2Action(ctx, opname);
    if (action) {
        return action->dispatch(ctx);
    }

    qCCritical(CUTELYST_DISPATCHER) << "Action not found" << action;
    return false;
}

void Dispatcher::prepareAction(Context *ctx)
{
    Q_D(Dispatcher);

    Request *request = ctx->request();
    QByteArray path = request->path().toLatin1();
    QList<QByteArray> pathParts = path.split('/');
    QStringList args;

    // Root action
    pathParts.prepend(QByteArrayLiteral(""));

    int pos = path.size();
    while (pos != -1) {
        QByteArray actionPath = path.mid(1, pos);
        Q_FOREACH (DispatchType *type, d->dispatchers) {
            if (type->match(ctx, actionPath, args)) {
                request->d_ptr->args = args;

                if (!path.isEmpty()) {
                    qCDebug(CUTELYST_DISPATCHER) << "Path is" << actionPath;
                }

                if (!args.isEmpty()) {
                    qCDebug(CUTELYST_DISPATCHER) << "Arguments are" << args.join(QLatin1Char('/'));
                }

                return;
            }
        }

        pos = path.lastIndexOf('/', pos);
        if (pos > 0) {
            // Remove trailing '/'
            --pos;
        }

        args.prepend(QUrl::fromPercentEncoding(pathParts.takeLast()));
    }
}

Action *Dispatcher::getAction(const QByteArray &name, const QByteArray &ns) const
{
    Q_D(const Dispatcher);

    if (name.isEmpty()) {
        return 0;
    }

    QByteArray action = cleanNamespace(ns);
    action.reserve(action.size() + name.size() + 1);
    action.append('/');
    action.append(name);

    return d->actionHash.value(action);
}

ActionList Dispatcher::getActions(const QByteArray &name, const QByteArray &ns) const
{
    Q_D(const Dispatcher);

    ActionList ret;
    if (name.isEmpty()) {
        return ret;
    }

    QByteArray _ns = cleanNamespace(ns);

    ActionList containers = d->getContainers(_ns);
    Q_FOREACH (Action *action, containers) {
        if (action->name() == name) {
            ret.prepend(action);
        }
    }

    return ret;
}

QHash<QByteArray, Controller *> Dispatcher::controllers() const
{
    Q_D(const Dispatcher);
    return d->constrollerHash;
}

QByteArray Dispatcher::uriForAction(Action *action, const QStringList &captures)
{
    Q_D(const Dispatcher);
    Q_FOREACH (DispatchType *dispatch, d->dispatchers) {
        QByteArray uri = dispatch->uriForAction(action, captures);
        if (!uri.isNull()) {
            return uri.isEmpty() ? QByteArray("/", 1) : uri;
        }
    }
    return QByteArray();
}

void Dispatcher::registerDispatchType(DispatchType *dispatchType)
{
    Q_D(Dispatcher);
    d->dispatchers.append(dispatchType);
}

QByteArray Dispatcher::printActions()
{
    Q_D(Dispatcher);

    QByteArray buffer;
    QTextStream out(&buffer, QIODevice::WriteOnly);
    bool showInternalActions = false;

    out << "Loaded Private actions:" << endl;
    QByteArray privateTitle("Private");
    QByteArray classTitle("Class");
    QByteArray methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QHash<QByteArray, Action*>::ConstIterator it = d->actionHash.constBegin();
    while (it != d->actionHash.constEnd()) {
        Action *action = it.value();
        QByteArray path = it.key();
        if (!path.startsWith('/')) {
            path.prepend('/');
        }
        privateLength = qMax(privateLength, path.length());
        classLength = qMax(classLength, action->className().length());
        actionLength = qMax(actionLength, action->name().length());
        ++it;
    }

    out << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
        << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
        << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
        << "." << endl;
    out << "|" << privateTitle.leftJustified(privateLength).data()
        << "|" << classTitle.leftJustified(classLength).data()
        << "|" << methodTitle.leftJustified(actionLength).data()
        << "|" << endl;
    out << "." << QByteArray().fill('-', privateLength).data()
        << "+" << QByteArray().fill('-', classLength).data()
        << "+" << QByteArray().fill('-', actionLength).data()
        << "." << endl;

    QList<QByteArray> keys = d->actionHash.keys();
    qSort(keys.begin(), keys.end());
    Q_FOREACH (const QByteArray &key, keys) {
        Action *action = d->actionHash.value(key);
        if (showInternalActions || !action->name().startsWith('_')) {
            QByteArray path = key;
            if (!path.startsWith('/')) {
                path.prepend('/');
            }
            out << "|" << path.leftJustified(privateLength).data()
                << "|" << action->className().leftJustified(classLength).data()
                << "|" << action->name().leftJustified(actionLength).data()
                << "|" << endl;
        }
    }

    out << "." << QByteArray().fill('-', privateLength).data()
        << "+" << QByteArray().fill('-', classLength).data()
        << "+" << QByteArray().fill('-', actionLength).data()
        << "." << endl;

    // List all public actions
    Q_FOREACH (DispatchType *dispatch, d->dispatchers) {
        out << endl << dispatch->list();
    }

    return buffer;
}

Action *Dispatcher::command2Action(Context *ctx, const QByteArray &command, const QStringList &extraParams)
{
    Q_D(Dispatcher);
//    qDebug() << Q_FUNC_INFO << "Command" << command;

    Action *ret = d->actionHash.value(command);
    if (!ret) {
        ret = invokeAsPath(ctx, command, ctx->args());
    }

    return ret;
}

QByteArray Dispatcher::actionRel2Abs(Context *ctx, const QByteArray &path)
{
    QByteArray ret = path;
    if (!ret.startsWith('/')) {
        // TODO at Catalyst it uses
        // c->stack->last()->namespace
        ret.prepend('/');
        ret.prepend(ctx->action()->ns());
    }

    if (ret.startsWith('/')) {
        ret.remove(0, 1);
    }

    return ret;
}

Action *Dispatcher::invokeAsPath(Context *ctx, const QByteArray &relativePath, const QStringList &args)
{
    Action *ret;
    QByteArray path = actionRel2Abs(ctx, relativePath);

    int pos = path.lastIndexOf('/');
    int lastPos = path.size();
    do {
        if (pos == -1) {
            ret = getAction(path, QByteArray());
            if (ret) {
                return ret;
            }
        } else {
            QByteArray name = path.mid(pos + 1, lastPos);
            path = path.mid(0, pos);
            ret = getAction(name, path);
            if (ret) {
                return ret;
            }
        }

        lastPos = pos;
        pos = path.indexOf('/', pos - 1);
    } while (pos != -1);

    return 0;
}

QByteArray Dispatcher::cleanNamespace(const QByteArray &ns) const
{
    QByteArray ret = ns;
    bool lastWasSlash = true; // remove initial slash
    int nsSize = ns.size();
    const char * data = ret.constData();
    for (int i = 0; i < nsSize; ++i) {
        if (data[i] == '/') {
            if (lastWasSlash) {
                ret.remove(i, 1);
                data = ret.constData();
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


ActionList DispatcherPrivate::getContainers(const QByteArray &ns) const
{
    ActionList ret;

    if (ns != "/") {
        int pos = ns.size();
//        qDebug() << pos << _ns.mid(0, pos);
        while (pos > 0) {
//            qDebug() << pos << _ns.mid(0, pos);
            ret.append(containerHash.value(ns.mid(0, pos)));
            pos = ns.lastIndexOf('/', pos - 1);
        }
    }   
    ret.append(containerHash.value(""));

    return ret;
}
