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

#include "cutelystdispatcher_p.h"

#include "context.h"
#include "cutelystcontroller.h"
#include "cutelystaction.h"
#include "cutelystrequest_p.h"
#include "cutelystdispatchtypepath.h"
#include "cutelystdispatchtypeindex.h"
#include "cutelystdispatchtypedefault.h"

#include <QUrl>
#include <QMetaMethod>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QDebug>

#include <iostream>

using namespace std;
using namespace Cutelyst;

CutelystDispatcher::CutelystDispatcher(QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystDispatcherPrivate)
{
    Q_D(CutelystDispatcher);
    d->dispatchers << new CutelystDispatchTypePath(this);
    d->dispatchers << new CutelystDispatchTypeIndex(this);
    d->dispatchers << new CutelystDispatchTypeDefault(this);
}

CutelystDispatcher::~CutelystDispatcher()
{
    delete d_ptr;
}

void CutelystDispatcher::setupActions()
{
    Q_D(CutelystDispatcher);

    // Find all the User classes
    for (int metaType = QMetaType::User; QMetaType::isRegistered(metaType); ++metaType) {
        qDebug() << "Type name:" << QMetaType::typeName(metaType);
        const QMetaObject *meta = QMetaType::metaObjectForType(metaType);
        if (meta && qstrcmp(meta->superClass()->className(), "CutelystController") == 0) {
            // App controller
            CutelystController *controller = qobject_cast<CutelystController*>(meta->newInstance());
            if (controller) {
                qDebug() << "Found a controller:" << controller << meta->className();
            } else {
                qWarning() << "Could not instantiate controller:" << meta->className();
                continue;
            }
            controller->setObjectName(meta->className());

            bool controllerUsed = false;
            for (int i = 0; i < meta->methodCount(); ++i) {
                QMetaMethod method = meta->method(i);
                if (method.methodType() == QMetaMethod::Method) {
//                    qDebug() << Q_FUNC_INFO << method.name() << method.attributes() << method.methodType() << method.methodSignature();
//                    qDebug() << Q_FUNC_INFO << method.parameterTypes() << method.tag() << method.access();
                    CutelystAction *action = new CutelystAction(method, controller);
                    if (action->isValid() && !d->actionHash.contains(action->privateName())) {
                        d->actionHash.insert(action->privateName(), action);
                        d->containerHash[action->ns()] << action;
                        controllerUsed = true;

                        if (!action->attributes().contains(QLatin1String("Private"))) {
                            bool registered = false;

                            // Register the action with each dispatcher
                            foreach (CutelystDispatchType *dispatch, d->dispatchers) {
                                if (dispatch->registerAction(action)) {
                                    registered = true;
                                }
                            }

                            if (!registered) {
                                qWarning() << "***Could NOT register the action" << action->name() << "with any dispatcher";
                            }
                        }
                    } else {
                        delete action;
                    }
                }
            }

            if (controllerUsed) {
                d->constrollerHash.insert(meta->className(), controller);
            } else {
                delete controller;
            }
        }
    }

    printActions();
}

bool CutelystDispatcher::dispatch(Context *ctx)
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

bool CutelystDispatcher::forward(Context *ctx, const QString &opname, const QStringList &arguments)
{
    CutelystAction *action = command2Action(ctx, opname);
    if (action) {
        return action->dispatch(ctx);
    }

    qCritical() << "Action not found" << action;
    return false;
}

void CutelystDispatcher::prepareAction(Context *ctx)
{
    Q_D(CutelystDispatcher);

    QString path = ctx->req()->path();
    QStringList pathParts = path.split(QLatin1Char('/'));
    QStringList args;
    CutelystDispatchType *dispatch = 0;

    // Take off the root action
    pathParts.takeFirst();

    while (!pathParts.isEmpty()) {
        path = pathParts.join(QLatin1Char('/'));
        if (path.startsWith(QLatin1Char('/'))) {
            path.remove(0, 1);
        }

        foreach (CutelystDispatchType *type, d->dispatchers) {
            if (type->match(ctx, path)) {
                dispatch = type;
                break;
            }
        }

        if (dispatch) {
            break;
        }

        args.prepend(pathParts.takeLast());
        ctx->req()->d_ptr->args = unexcapedArgs(args);

    }

    if (!path.isEmpty()) {
        qDebug() << "Path is" << path;
    }

    if (!ctx->args().isEmpty()) {
        qDebug() << "Arguments are" << ctx->args().join(QLatin1Char('/'));
    }
}

CutelystAction *CutelystDispatcher::getAction(const QString &name, const QString &ns) const
{
    Q_D(const CutelystDispatcher);
    if (name.isEmpty()) {
        return 0;
    }

    QString _ns = cleanNamespace(ns);

    return d->actionHash.value(_ns % QLatin1Char('/') % name);
}

ActionList CutelystDispatcher::getActions(const QString &name, const QString &ns) const
{
    Q_D(const CutelystDispatcher);

    ActionList ret;
    if (name.isEmpty()) {
        return ret;
    }

    QString _ns = cleanNamespace(ns);

    ActionList containers = d->getContainers(_ns);
    foreach (CutelystAction *action, containers) {
        if (action->name() == name) {
            ret.prepend(action);
        }
    }

    return ret;
}

QHash<QString, CutelystController *> CutelystDispatcher::controllers() const
{
    Q_D(const CutelystDispatcher);
    return d->constrollerHash;
}

QString CutelystDispatcher::uriForAction(CutelystAction *action, const QStringList &captures)
{
    Q_D(const CutelystDispatcher);
    foreach (CutelystDispatchType *dispatch, d->dispatchers) {
        QString uri = dispatch->uriForAction(action, captures);
        if (!uri.isNull()) {
            return uri.isEmpty() ? QLatin1String("/") : uri;
        }
    }
    return QString();
}

void CutelystDispatcher::printActions()
{
    Q_D(CutelystDispatcher);

    bool showInternalActions = true;
    cout << "Loaded Private actions:" << endl;
    QString privateTitle("Private");
    QString classTitle("Class");
    QString methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QMap<QString, CutelystAction*>::ConstIterator it = d->actionHash.constBegin();
    while (it != d->actionHash.constEnd()) {
        CutelystAction *action = it.value();
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
        CutelystAction *action = it.value();
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

CutelystAction *CutelystDispatcher::command2Action(Context *ctx, const QString &command, const QStringList &extraParams)
{
    Q_D(CutelystDispatcher);
//    qDebug() << Q_FUNC_INFO << "Command" << command;

    CutelystAction *ret = d->actionHash.value(command);
    if (!ret) {
        ret = invokeAsPath(ctx, command, ctx->args());
    }

    return ret;
}

QStringList CutelystDispatcher::unexcapedArgs(const QStringList &args)
{
    QStringList ret;
    foreach (const QString &arg, args) {
        ret << QUrl::fromPercentEncoding(arg.toLocal8Bit());
    }
    return ret;
}

QString CutelystDispatcher::actionRel2Abs(Context *ctx, const QString &path)
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

CutelystAction *CutelystDispatcher::invokeAsPath(Context *ctx, const QString &relativePath, const QStringList &args)
{
    Q_D(CutelystDispatcher);

    CutelystAction *ret = 0;
    QString path = actionRel2Abs(ctx, relativePath);

    QRegularExpression re("^(?:(.*)/)?(\\w+)?$");
    while (!path.isEmpty()) {
        QRegularExpressionMatch match = re.match(path);
        if (match.hasMatch()) {
            path = match.captured(1);
            const QString &tail = match.captured(2);
            if (ret = getAction(tail, path)) {
                break;
            }
        } else {
            break;
        }
    }

    return ret;
}

QString CutelystDispatcher::cleanNamespace(const QString &ns) const
{
    QStringList ret;
    foreach (const QString &part, ns.split(QLatin1Char('/'))) {
        if (!part.isEmpty()) {
            ret << part;
        }
    }
    return ret.join(QLatin1Char('/'));
}


ActionList CutelystDispatcherPrivate::getContainers(const QString &ns) const
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
