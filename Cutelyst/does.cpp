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

#include "does_p.h"

using namespace Cutelyst;

Does::Does() :
    d_ptr(new DoesPrivate)
{
}

Does::~Does()
{
    delete d_ptr;
}

bool Does::init(const QVariantHash &args)
{
    Q_UNUSED(args)
    return true;
}

bool Does::execute(Context *ctx)
{
    Q_D(Does);

    if (d->proccessRoles) {
        Q_FOREACH (Does *does, d->beforeRoles) {
            if (!does->beforeExecute(ctx)) {
                return false;
            }
        }

        DoesCode code;
        code.execute = this;
        code.stack = d->aroundRoles;
        if (!aroundExecute(ctx, code)) {
            return false;
        }

        Q_FOREACH (Does *does, d->afterRoles) {
            if (!does->afterExecute(ctx)) {
                return false;
            }
        }

        // Do not call doExecute twice
        return true;
    }

    return doExecute(ctx);
}

bool Does::beforeExecute(Context *ctx) const
{
    Q_UNUSED(ctx)
    return true;
}

bool Does::aroundExecute(Context *ctx, Cutelyst::Does::DoesCode code) const
{
    Q_UNUSED(ctx)

    if (!code.stack.isEmpty()) {
        Does *does = code.stack.pop();
        return does->aroundExecute(ctx, code);
    }

    return code.execute->doExecute(ctx);
}

bool Does::afterExecute(Context *ctx) const
{
    Q_UNUSED(ctx)
    return true;
}

bool Does::doExecute(Context *ctx) const
{
    Q_UNUSED(ctx)
    return true;
}

void Does::applyRoles(const QStack<Cutelyst::Does *> &roles)
{
    Q_D(Does);

    for (int i = 0; i < roles.size(); ++i) {
        Does *does = roles.at(i);
        if (does->modifiers() & BeforeExecute) {
            d->beforeRoles.push(does);
        }

        if (does->modifiers() & AroundExecute) {
            d->aroundRoles.push(does);
        }

        if (does->modifiers() & AfterExecute) {
            d->afterRoles.push(does);
        }
    }
    d->roles = roles;
    d->proccessRoles = true;
}

bool Does::dispatcherReady(const Dispatcher *dispatch, Controller *controller)
{
    Q_D(Does);

    for (int i = 0; i < d->roles.size(); ++i) {
        Does *does = d->roles.at(i);
        does->dispatcherReady(dispatch, controller);
    }
    return true;
}
