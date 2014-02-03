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

#include "context.h"
#include "controller.h"
#include "action.h"
#include "request_p.h"
#include "dispatchtypepath.h"

#include <QUrl>
#include <QMetaMethod>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QDebug>

#include <iostream>

using namespace std;
using namespace Cutelyst;

Dispatcher::Dispatcher(QObject *parent) :
    QObject(parent),
    d_ptr(new DispatcherPrivate)
{
    Q_D(Dispatcher);
    d->dispatchers << new DispatchTypePath(this);
}

Dispatcher::~Dispatcher()
{
    delete d_ptr;
}

void Dispatcher::setupActions(const QList<Controller*> &controllers)
{
    Q_D(Dispatcher);

    foreach (Controller *controller, controllers) {
        // App controller
//        qDebug() << "Found a controller:" << controller << meta->className();
        const QMetaObject *meta = controller->metaObject();
        controller->setObjectName(meta->className());
        bool instanceUsed = false;
        for (int i = 0; i < meta->methodCount(); ++i) {
            QMetaMethod method = meta->method(i);
            if (method.methodType() == QMetaMethod::Method) {
                //                    qDebug() << Q_FUNC_INFO << method.name() << method.attributes() << method.methodType() << method.methodSignature();
                //                    qDebug() << Q_FUNC_INFO << method.parameterTypes() << method.tag() << method.access();
                bool registered = false;
                Action *action = new Action(method, controller);
                if (action->isValid() && !d->actionHash.contains(action->privateName())) {
                    if (!action->attributes().contains("Private")) {
                        // Register the action with each dispatcher
                        foreach (CutelystDispatchType *dispatch, d->dispatchers) {
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

    printActions();
}

bool Dispatcher::dispatch(Context *ctx)
{
    if (ctx->action()) {
        return ctx->forward(QLatin1Char('/') % ctx->action()->ns() % QLatin1String("/_DISPATCH"));
    } else {
        QString error;
        QString path = ctx->req()->path();
        if (path.isEmpty()) {
            error = QLatin1String("No default action defined");
        } else {
            error = QLatin1String("Unknown resource \"") % path % QLatin1Char('"');
        }
        qDebug() << Q_FUNC_INFO << error;
        ctx->error(error);
    }
    return false;
}

bool Dispatcher::forward(Context *ctx, const QString &opname, const QStringList &arguments)
{
    Action *action = command2Action(ctx, opname);
    if (action) {
        return action->dispatch(ctx);
    }

    qCritical() << "Action not found" << action;
    return false;
}

void Dispatcher::prepareAction(Context *ctx)
{
    Q_D(Dispatcher);

    QString path = ctx->req()->path();
    QStringList pathParts = path.split(QLatin1Char('/'));
    QStringList args;

    // Root action
    pathParts.prepend(QLatin1String(""));

    while (!pathParts.isEmpty()) {
        path = pathParts.join(QLatin1Char('/'));
        path.remove(QRegularExpression("^/+"));

        foreach (CutelystDispatchType *type, d->dispatchers) {
            if (type->match(ctx, path)) {
                if (!path.isEmpty()) {
                    qDebug() << "Path is" << path;
                }

                if (!ctx->args().isEmpty()) {
                    qDebug() << "Arguments are" << ctx->args().join(QLatin1Char('/'));
                }

                return;
            }
        }

        args.prepend(pathParts.takeLast());
        ctx->req()->d_ptr->args = unexcapedArgs(args);
    }
}

Action *Dispatcher::getAction(const QString &name, const QString &ns) const
{
    Q_D(const Dispatcher);
    if (name.isEmpty()) {
        return 0;
    }

    QString _ns = cleanNamespace(ns);

    return d->actionHash.value(_ns % QLatin1Char('/') % name);
}

ActionList Dispatcher::getActions(const QString &name, const QString &ns) const
{
    Q_D(const Dispatcher);

    ActionList ret;
    if (name.isEmpty()) {
        return ret;
    }

    QString _ns = cleanNamespace(ns);

    ActionList containers = d->getContainers(_ns);
    foreach (Action *action, containers) {
        if (action->name() == name) {
            ret.prepend(action);
        }
    }

    return ret;
}

QHash<QString, Controller *> Dispatcher::controllers() const
{
    Q_D(const Dispatcher);
    return d->constrollerHash;
}

QString Dispatcher::uriForAction(Action *action, const QStringList &captures)
{
    Q_D(const Dispatcher);
    foreach (CutelystDispatchType *dispatch, d->dispatchers) {
        QString uri = dispatch->uriForAction(action, captures);
        if (!uri.isNull()) {
            return uri.isEmpty() ? QLatin1String("/") : uri;
        }
    }
    return QString();
}

void Dispatcher::printActions()
{
    Q_D(Dispatcher);

    bool showInternalActions = false;
    cout << "Loaded Private actions:" << endl;
    QString privateTitle("Private");
    QString classTitle("Class");
    QString methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QMap<QString, Action*>::ConstIterator it = d->actionHash.constBegin();
    while (it != d->actionHash.constEnd()) {
        Action *action = it.value();
        QString path = it.key();
        if (!path.startsWith(QLatin1String("/"))) {
            path.prepend(QLatin1String("/"));
        }
        privateLength = qMax(privateLength, path.length());
        classLength = qMax(classLength, action->className().length());
        actionLength = qMax(actionLength, action->name().length());
        ++it;
    }

    cout << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
         << "." << endl;
    cout << "|" << privateTitle.leftJustified(privateLength).toUtf8().data()
         << "|" << classTitle.leftJustified(classLength).toUtf8().data()
         << "|" << methodTitle.leftJustified(actionLength).toUtf8().data()
         << "|" << endl;
    cout << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
         << "." << endl;

    it = d->actionHash.constBegin();
    while (it != d->actionHash.constEnd()) {
        Action *action = it.value();
        if (showInternalActions || !action->name().startsWith(QLatin1Char('_'))) {
            QString path = it.key();
            if (!path.startsWith(QLatin1String("/"))) {
                path.prepend(QLatin1String("/"));
            }
            cout << "|" << path.leftJustified(privateLength).toUtf8().data()
                 << "|" << action->className().leftJustified(classLength).toUtf8().data()
                 << "|" << action->name().leftJustified(actionLength).toUtf8().data()
                 << "|" << endl;
        }
        ++it;
    }

    cout << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
         << "."  << endl << endl;

    // List all public actions
    foreach (CutelystDispatchType *dispatch, d->dispatchers) {
        dispatch->list();
    }
}

Action *Dispatcher::command2Action(Context *ctx, const QString &command, const QStringList &extraParams)
{
    Q_D(Dispatcher);
//    qDebug() << Q_FUNC_INFO << "Command" << command;

    Action *ret = d->actionHash.value(command);
    if (!ret) {
        ret = invokeAsPath(ctx, command, ctx->args());
    }

    return ret;
}

QStringList Dispatcher::unexcapedArgs(const QStringList &args)
{
    QStringList ret;
    foreach (const QString &arg, args) {
        ret << QUrl::fromPercentEncoding(arg.toLocal8Bit());
    }
    return ret;
}

QString Dispatcher::actionRel2Abs(Context *ctx, const QString &path)
{
    QString ret = path;
    if (!ret.startsWith(QLatin1Char('/'))) {
        // TODO at Catalyst it uses
        // c->stack->last()->namespace
        QString ns = ctx->action()->ns();
        ret = ns % QLatin1Char('/') % path;
    }

    if (ret.startsWith(QLatin1Char('/'))) {
        ret.remove(0, 1);
    }

    return ret;
}

Action *Dispatcher::invokeAsPath(Context *ctx, const QString &relativePath, const QStringList &args)
{
    Action *ret = 0;
    QString path = actionRel2Abs(ctx, relativePath);

    QRegularExpression re("^(?:(.*)/)?(\\w+)?$");
    while (!path.isEmpty()) {
        QRegularExpressionMatch match = re.match(path);
        if (match.hasMatch()) {
            path = match.captured(1);
            ret = getAction(match.captured(2), path);
            if (ret) {
                break;
            }
        } else {
            break;
        }
    }

    return ret;
}

QString Dispatcher::cleanNamespace(const QString &ns) const
{
    QStringList ret;
    foreach (const QString &part, ns.split(QLatin1Char('/'))) {
        if (!part.isEmpty()) {
            ret << part;
        }
    }
    return ret.join(QLatin1Char('/'));
}


ActionList DispatcherPrivate::getContainers(const QString &ns) const
{
    ActionList ret;

    QString _ns = ns;
    if (_ns == QLatin1String("/")) {
        _ns = QLatin1String("");
    }

    while (!_ns.isEmpty()) {
        ret << containerHash.value(_ns);
        _ns = _ns.section(QLatin1Char('/'), 0, -2);
    }
    ret << containerHash.value(_ns);

    return ret;
}
