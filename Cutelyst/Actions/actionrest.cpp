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

#include "actionrest.h"
#include "context.h"
#include "controller.h"
#include "dispatcher.h"

#include <QDebug>

using namespace Cutelyst;

ActionREST::ActionREST()
{
    qDebug() << Q_FUNC_INFO;
}

bool ActionREST::dispatch(Context *ctx) const
{
    bool ret = Action::dispatch(ctx);
    if (!ret) {
        return false;
    }

    QByteArray restMethod = name() + '_' + ctx->req()->method();
    const Action *action = controller()->actionFor(restMethod);
    if (action) {
        return action->dispatch(ctx);
    }

    return false;
}

void ActionREST::dispatcherReady(const Dispatcher *dispatch)
{
//    qDebug() << name() << dispatch;
}
