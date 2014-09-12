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

#include "actionrest_p.h"
#include "context.h"
#include "controller.h"
#include "dispatcher.h"

#include <QDebug>

using namespace Cutelyst;

ActionREST::ActionREST() :
    d_ptr(new ActionRESTPrivate)
{
    d_ptr->q_ptr = this;
    qDebug() << Q_FUNC_INFO;
}

ActionREST::~ActionREST()
{
    delete d_ptr;
}

bool ActionREST::dispatch(Context *ctx) const
{
    Q_D(const ActionREST);

    bool ret = Action::dispatch(ctx);
    if (!ret) {
        return false;
    }

    return d->dispatchRestMethod(ctx, ctx->request()->method());
}

void ActionREST::dispatcherReady(const Dispatcher *dispatch)
{
//    qDebug() << name() << dispatch;
}


bool ActionRESTPrivate::dispatchRestMethod(Context *ctx, const QByteArray &httpMethod) const
{
    Q_Q(const ActionREST);
    const QByteArray &restMethod = q->name() + '_' + httpMethod;

    Controller *controller = ctx->controller();
    const Action *action = controller->actionFor(restMethod);
    if (!action) {
        // Look for non registered actions in this controller
        ActionList actions = controller->actions();
        Q_FOREACH (Action *controllerAction, actions) {
            if (controllerAction->name() == restMethod) {
                action = controllerAction;
                break;
            }
        }
    }

    if (action) {
        return action->dispatch(ctx);
    }

    bool ret = false;
    if (httpMethod == "OPTIONS") {
        ret = returnOptions(ctx, restMethod);
    } else if (httpMethod == "HEAD") {
        // redispatch to GET
        ret = dispatchRestMethod(ctx, QByteArrayLiteral("GET"));
    } else if (httpMethod != "not_implemented") {
        // try dispatching to foo_not_implemented
        ret = dispatchRestMethod(ctx, QByteArrayLiteral("not_implemented"));
    } else {
        // not_implemented
        ret = returnNotImplemented(ctx, q->name());
    }

    return ret;
}

bool ActionRESTPrivate::returnOptions(Context *ctx, const QByteArray &methodName) const
{
    Response *response = ctx->response();
    response->setContentType(QByteArrayLiteral("text/plain"));
    response->setStatus(Response::OK); // 200
    response->headers().insert(QByteArrayLiteral("Allow"),
                               getAllowedMethods(ctx->controller(), ctx, methodName));
    response->body().clear();
    return true;
}

bool ActionRESTPrivate::returnNotImplemented(Context *ctx, const QByteArray &methodName) const
{
    Response *response = ctx->response();
    response->setContentType(QByteArrayLiteral("text/plain"));
    response->setStatus(Response::MethodNotAllowed); // 405
    response->headers().insert(QByteArrayLiteral("Allow"),
                               getAllowedMethods(ctx->controller(), ctx, methodName));
    response->body() = "Method " + ctx->req()->method() + " not implemented for " + ctx->uriFor(methodName);
    return true;
}

QByteArray Cutelyst::ActionRESTPrivate::getAllowedMethods(Controller *controller, Context *ctx, const QByteArray &methodName) const
{
    return QByteArray();
}
