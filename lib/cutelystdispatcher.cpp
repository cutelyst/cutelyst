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

#include "cutelystcontext.h"
#include "cutelystcontroller.h"
#include "cutelystaction.h"
#include "cutelystrequest_p.h"
#include "cutelystdispatchtypepath.h"

#include <QUrl>
#include <QMetaMethod>
#include <QStringBuilder>
#include <QDebug>

CutelystDispatcher::CutelystDispatcher(QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystDispatcherPrivate)
{
    Q_D(CutelystDispatcher);
    d->dispatchers << new CutelystDispatchTypePath(this);
}

CutelystDispatcher::~CutelystDispatcher()
{
    qDebug() << Q_FUNC_INFO;
    delete d_ptr;
}

void CutelystDispatcher::setupActions()
{
    Q_D(CutelystDispatcher);

    // Find all the User classes
    int metaType = QMetaType::User;
    while (QMetaType::isRegistered(metaType)) {
        const QMetaObject *meta = QMetaType::metaObjectForType(metaType);
        if (qstrcmp(meta->superClass()->className(), "CutelystController") == 0) {
            // App controller
            CutelystController *controller = qobject_cast<CutelystController*>(meta->newInstance());
            qDebug() << "Found a controller:" << controller << meta->className();

            int i = 0;
            while (i < meta->methodCount()) {
                QMetaMethod method = meta->method(i);
                if (method.methodType() == QMetaMethod::Method) {
//                    qDebug() << Q_FUNC_INFO << method.name() << method.attributes() << method.methodType() << method.methodSignature();
//                    qDebug() << Q_FUNC_INFO << method.parameterTypes() << method.tag() << method.access();
                    CutelystAction *action = new CutelystAction(method, controller);

                    if (!d->actions.contains(action->privateName())) {
                        d->actions.insert(action->privateName(), action);

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
                ++i;
            }
        }

        if (meta->classInfoCount()) {
            QMetaClassInfo classInfo = meta->classInfo(0);
            qDebug() << Q_FUNC_INFO << classInfo.name() << classInfo.value();
        }
        ++metaType;
    }

    printActions();
}

void CutelystDispatcher::dispatch(CutelystContext *c)
{
    if (c->action()) {
        c->forward(QLatin1Char('/') % c->action()->ns() % QLatin1String("/_DISPATCH"));
    }
}

bool CutelystDispatcher::forward(CutelystContext *c, const QString &opname, const QStringList &arguments)
{
    CutelystAction *action = command2Action(c, opname);
    qDebug() << Q_FUNC_INFO << opname << action;
    if (action) {
        qDebug() << Q_FUNC_INFO << action->name();
        return action->dispatch(c);
    } else {
        qWarning() << "Action not found" << action;
    }
    return false;
}

void CutelystDispatcher::prepareAction(CutelystContext *c)
{
    Q_D(CutelystDispatcher);

    QString path = c->req()->path();
    QStringList pathParts = path.split(QLatin1Char('/'));
    QStringList args;
    CutelystDispatchType *dispatch = 0;

    while (!pathParts.isEmpty()) {
        path = pathParts.join(QLatin1Char('/'));
        if (path.startsWith(QLatin1Char('/'))) {
            path.remove(0, 1);
        }

        foreach (CutelystDispatchType *type, d->dispatchers) {
            if (type->match(c, path)) {
                qDebug() << Q_FUNC_INFO << "Found a dispatcher type" << type;
                dispatch = type;
                break;
            }
        }

        if (dispatch) {
            break;
        }

        args.prepend(pathParts.takeLast());
        c->req()->d_ptr->args = unexcapedArgs(args);

    }

    qDebug() << Q_FUNC_INFO << "Path is " << path;
    qDebug() << Q_FUNC_INFO << "Arguments are " << c->args().join(QLatin1Char('/'));
}

CutelystAction *CutelystDispatcher::getAction(const QString &action, const QString &ns)
{

}

QList<CutelystAction *> CutelystDispatcher::getActions(const QString &action, const QString &ns)
{

}

void CutelystDispatcher::printActions()
{
    Q_D(CutelystDispatcher);

    bool showInternalActions = true;
    qDebug() << "Loaded Private actions:";
    QString privateTitle("Private");
    QString classTitle("Class");
    QString methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QHash<QString, CutelystAction*>::ConstIterator it = d->actions.constBegin();
    while (it != d->actions.constEnd()) {
        CutelystAction *action = it.value();
        privateLength = qMax(privateLength, it.key().length());
        classLength = qMax(classLength, action->className().length());
        actionLength = qMax(actionLength, action->name().length());
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";
    qDebug() << "|" << privateTitle.leftJustified(privateLength).toUtf8().data()
             << "|" << classTitle.leftJustified(classLength).toUtf8().data()
             << "|" << methodTitle.leftJustified(actionLength).toUtf8().data()
             << "|";
    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";

    it = d->actions.constBegin();
    while (it != d->actions.constEnd()) {
        CutelystAction *action = it.value();
        if (showInternalActions || !action->name().startsWith(QLatin1Char('_'))) {
            qDebug() << "|" << it.key().leftJustified(privateLength).toUtf8().data()
                     << "|" << action->className().leftJustified(classLength).toUtf8().data()
                     << "|" << action->name().leftJustified(actionLength).toUtf8().data()
                     << "|";
        }
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".\n";

    // List all public actions
    foreach (CutelystDispatchType *dispatch, d->dispatchers) {
        dispatch->list();
    }
}

CutelystAction *CutelystDispatcher::command2Action(CutelystContext *c, const QString &command, const QStringList &extraParams)
{
    Q_D(CutelystDispatcher);
    qDebug() << Q_FUNC_INFO << command;
    printActions();

    if (d->actions.contains(command)) {
        qDebug() << Q_FUNC_INFO << command;

        return d->actions.value(command);
    }
    return 0;
}

QStringList CutelystDispatcher::unexcapedArgs(const QStringList &args)
{
    QStringList ret;
    foreach (const QString &arg, args) {
        ret << QUrl::fromPercentEncoding(arg.toLocal8Bit());
    }
    return ret;
}
