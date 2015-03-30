/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "code_p.h"
#include "common.h"

#include <QStringBuilder>

using namespace Cutelyst;

Code::Code(QObject *parent) :
    QObject(parent),
    d_ptr(new CodePrivate)
{
    if (objectName().isNull()) {
        setObjectName(this->metaObject()->className() % QLatin1String("->execute"));
    }
}

Code::~Code()
{
    delete d_ptr;
}

QString Code::name() const
{
    Q_D(const Code);
    return d->name;
}

void Code::setName(const QString &name)
{
    Q_D(Code);
    d->name = name;
}

bool Code::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_UNUSED(application)
    Q_UNUSED(args)
    return true;
}

bool Code::execute(Context *ctx)
{
    Q_D(Code);

    if (d->proccessRoles) {
        Q_FOREACH (Code *code, d->beforeRoles) {
            if (!code->beforeExecute(ctx)) {
                return false;
            }
        }

        QStack<Code *> stack = d->aroundRoles;
        // first item on the stack is always the execution code
        stack.push_front(this);
        if (!aroundExecute(ctx, stack)) {
            return false;
        }

        Q_FOREACH (Code *code, d->afterRoles) {
            if (!code->afterExecute(ctx)) {
                return false;
            }
        }

        // Do not call doExecute twice
        return true;
    }

    return doExecute(ctx);
}

bool Code::beforeExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

bool Code::aroundExecute(Context *ctx, QStack<Cutelyst::Code *> stack)
{
    Q_UNUSED(ctx)

    int stackSize = stack.size();
    if (stackSize == 1) {
        Code *code = stack.pop();
        return code->doExecute(ctx);
    } else if (stackSize > 1) {
        Code *code = stack.pop();
        return code->aroundExecute(ctx, stack);
    }

    // Should NEVER happen
    qCCritical(CUTELYST_CODE) << "Reached end of the stack!" << ctx->req()->uri();
    return false;
}

bool Code::afterExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

bool Code::doExecute(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

void Code::applyRoles(const QStack<Cutelyst::Code *> &roles)
{
    Q_D(Code);

    for (int i = 0; i < roles.size(); ++i) {
        Code *code = roles.at(i);
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

bool Code::dispatcherReady(const Dispatcher *dispatch, Controller *controller)
{
    Q_D(Code);

    for (int i = 0; i < d->roles.size(); ++i) {
        Code *code = d->roles.at(i);
        code->dispatcherReady(dispatch, controller);
    }
    return true;
}
