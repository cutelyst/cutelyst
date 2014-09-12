/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "controller_p.h"

#include "dispatcher.h"
#include "action.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QDebug>

using namespace Cutelyst;

Controller::Controller(QObject *parent) :
    QObject(parent),
    d_ptr(new ControllerPrivate)
{
}

Controller::~Controller()
{
    delete d_ptr;
}

QByteArray Controller::ns() const
{
    Q_D(const Controller);
    return d->ns;
}

const Action *Controller::actionFor(const QByteArray &name) const
{
    Q_D(const Controller);
    return d->dispatcher->getAction(name, d->ns);
}

ActionList Controller::actions() const
{
    Q_D(const Controller);
    return d->actions;
}

bool Controller::operator==(const char *className)
{
    return !qstrcmp(metaObject()->className(), className);
}

void Controller::Begin(Context *ctx)
{

}

bool Controller::Auto(Context *ctx)
{
    return true;
}

void Controller::End(Context *ctx)
{

}

void Controller::init()
{
    Q_D(Controller);

    const QMetaObject *meta = metaObject();
    const QString &className = QString::fromLatin1(meta->className());
    setObjectName(className);

    QByteArray controlerNS;
    for (int i = 0; i < meta->classInfoCount(); ++i) {
        if (metaObject()->classInfo(i).name() == QLatin1String("Namespace")) {
            controlerNS = meta->classInfo(i).value();
            break;
        }
    }

    if (controlerNS.isNull()) {
        bool lastWasUpper = true;

        for (int i = 0; i < className.length(); ++i) {
            if (className.at(i).toLower() == className.at(i)) {
                controlerNS.append(className.at(i));
                lastWasUpper = false;
            } else {
                if (lastWasUpper) {
                    controlerNS.append(className.at(i).toLower());
                } else {
                    controlerNS.append(QLatin1Char('/') % className.at(i).toLower());
                }
                lastWasUpper = true;
            }
        }
    }
    d->ns = controlerNS;

    // Setup actions
    for (int i = 0; i < meta->methodCount(); ++i) {
        const QMetaMethod &method = meta->method(i);
        // We register actions that are either a Q_SLOT
        // or a Q_INVOKABLE function which has the first
        // parameter type equal to Context*
        if (method.isValid() &&
                (method.methodType() == QMetaMethod::Method || method.methodType() == QMetaMethod::Slot) &&
                (method.parameterCount() && method.parameterType(0) == qMetaTypeId<Cutelyst::Context *>())) {

            Action *action = d->actionForMethod(method);
            action->setupAction(method, this);

            d->actions.append(action);
        }
    }
}

void Controller::setupActions(Dispatcher *dispatcher)
{
    Q_D(Controller);

    d->dispatcher = dispatcher;

    ActionList beginList;
    beginList = dispatcher->getActions(QByteArrayLiteral("Begin"), d->ns);
    if (!beginList.isEmpty()) {
        d->begin = beginList.last();
        d->actionSteps.append(d->begin);
    }

    d->autoList = dispatcher->getActions(QByteArrayLiteral("Auto"), d->ns);
    d->actionSteps.append(d->autoList);

    ActionList endList;
    endList = dispatcher->getActions(QByteArrayLiteral("End"), d->ns);
    if (!endList.isEmpty()) {
        d->end = endList.last();
    }
}

void Controller::_DISPATCH(Context *ctx)
{
    Q_D(Controller);

    bool failedState = false;

    // Dispatch to _BEGIN and _AUTO
    Q_FOREACH (Action *action, d->actionSteps) {
        if (!action->dispatch(ctx)) {
            failedState = true;
            break;
        }
    }

    // Dispatch to _ACTION
    if (!failedState) {
        ctx->action()->dispatch(ctx);
    }

    // Dispatch to _END
    if (d->end) {
        d->end->dispatch(ctx);
    }
}

bool Controller::_BEGIN(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    if (d->begin) {
        d->begin->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}

bool Controller::_AUTO(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    Q_FOREACH (Action *autoAction, d->autoList) {
        if (!autoAction->dispatch(ctx)) {
            return false;
        }
    }
    return true;
}

bool Controller::_ACTION(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    if (ctx->action()) {
        return ctx->action()->dispatch(ctx);
    }
    return !ctx->error();
}

bool Controller::_END(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    if (d->end) {
        d->end->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}


Action *ControllerPrivate::actionForMethod(const QMetaMethod &method)
{
    Action *ret = 0;

    for (int i = 1; i < method.parameterCount(); ++i) {
        int id = method.parameterType(i);
        if (id >= QMetaType::User) {
            const QMetaObject *metaObj = QMetaType::metaObjectForType(id);
            if (metaObj) {
                QObject *object = metaObj->newInstance();
                if (object && superIsAction(metaObj->superClass())) {
                    ret = qobject_cast<Action*>(object);
                    break;
                } else {
                    delete object;
                }
            }
        }
    }

    if (!ret) {
        ret = new Action;
    }

    return ret;
}

bool ControllerPrivate::superIsAction(const QMetaObject *super)
{
    if (super) {
        if (qstrcmp(super->className(), "Cutelyst::Action") == 0) {
            return true;
        }
        return superIsAction(super->superClass());
    }
    return false;
}
