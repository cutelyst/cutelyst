/*
 * Copyright (C) 2014-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "component_p.h"
#include "common.h"

#include <QStringBuilder>

using namespace Cutelyst;

Component::Component(QObject *parent) : QObject(parent)
  , d_ptr(new ComponentPrivate)
{
    if (objectName().isNull()) {
        setObjectName(this->metaObject()->className() % QLatin1String("->execute"));
    }
}

Component::~Component()
{
    delete d_ptr;
}

QString Component::name() const
{
    Q_D(const Component);
    return d->name;
}

void Component::setName(const QString &name)
{
    Q_D(Component);
    d->name = name;
}

bool Component::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_UNUSED(application)
    Q_UNUSED(args)
    return true;
}

bool Component::execute(Context *ctx)
{
    Q_D(Component);

    if (d->proccessRoles) {
        Q_FOREACH (Component *code, d->beforeRoles) {
            if (!code->beforeExecute(ctx)) {
                return false;
            }
        }

        QStack<Component *> stack = d->aroundRoles;
        // first item on the stack is always the execution code
        stack.push_front(this);
        if (!aroundExecute(ctx, stack)) {
            return false;
        }

        Q_FOREACH (Component *code, d->afterRoles) {
            if (!code->afterExecute(ctx)) {
                return false;
            }
        }

        // Do not call doExecute twice
        return true;
    }

    return doExecute(ctx);
}

bool Component::beforeExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

bool Component::aroundExecute(Context *ctx, QStack<Cutelyst::Component *> stack)
{
    Q_UNUSED(ctx)

    int stackSize = stack.size();
    if (stackSize == 1) {
        Component *code = stack.pop();
        return code->doExecute(ctx);
    } else if (stackSize > 1) {
        Component *code = stack.pop();
        return code->aroundExecute(ctx, stack);
    }

    // Should NEVER happen
    qCCritical(CUTELYST_COMPONENT) << "Reached end of the stack!" << ctx->req()->uri();
    return false;
}

bool Component::afterExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

bool Component::doExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

void Component::applyRoles(const QStack<Cutelyst::Component *> &roles)
{
    Q_D(Component);

    for (int i = 0; i < roles.size(); ++i) {
        Component *code = roles.at(i);
        if (code->modifiers() & BeforeExecute) {
            d->beforeRoles.push(code);
        }

        if (code->modifiers() & AroundExecute) {
            d->aroundRoles.push(code);
        }

        if (code->modifiers() & AfterExecute) {
            d->afterRoles.push(code);
        }
    }
    d->roles = roles;
    d->proccessRoles = true;
}

bool Component::dispatcherReady(const Dispatcher *dispatch, Controller *controller)
{
    Q_D(Component);

    for (int i = 0; i < d->roles.size(); ++i) {
        Component *code = d->roles.at(i);
        code->dispatcherReady(dispatch, controller);
    }
    return true;
}
