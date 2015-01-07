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

#include <QUrl>
#include <QDebug>

using namespace Cutelyst;

ActionREST::ActionREST() :
    d_ptr(new ActionRESTPrivate)
{
    d_ptr->q_ptr = this;
}

ActionREST::~ActionREST()
{
    delete d_ptr;
}

bool ActionREST::dispatch(Context *ctx)
{
    Q_D(const ActionREST);

    bool ret = Action::dispatch(ctx);
    if (!ret) {
        return false;
    }

    return d->dispatchRestMethod(ctx, ctx->request()->method());
}

bool ActionRESTPrivate::dispatchRestMethod(Context *ctx, const QString &httpMethod) const
{
    Q_Q(const ActionREST);
    const QByteArray &restMethod = q->name() + '_' + httpMethod.toLatin1();

    Controller *controller = ctx->controller();
    Action *action = controller->actionFor(restMethod);
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
        return ctx->execute(action);
    }

    bool ret = false;
    if (httpMethod == "OPTIONS") {
        ret = returnOptions(ctx, q->name());
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
                               getAllowedMethods(ctx->controller(), methodName));
    response->body().clear();
    return true;
}

bool ActionRESTPrivate::returnNotImplemented(Context *ctx, const QByteArray &methodName) const
{
    Response *response = ctx->response();
    response->setContentType(QByteArrayLiteral("text/plain"));
    response->setStatus(Response::MethodNotAllowed); // 405
    response->headers().insert(QByteArrayLiteral("Allow"),
                               getAllowedMethods(ctx->controller(), methodName));
    response->body() = "Method " + ctx->req()->method().toLatin1() + " not implemented for "
            + ctx->uriFor(methodName).toString().toLatin1();
    return true;
}

QByteArray Cutelyst::ActionRESTPrivate::getAllowedMethods(Controller *controller, const QByteArray &methodName) const
{
    QStringList methods;
    QByteArray name = methodName + '_';
    ActionList actions = controller->actions();
    Q_FOREACH (Action *action, actions) {
        const QString &method = action->name();
        if (method.startsWith(name)) {
            methods.append(method.mid(name.size()));
        }
    }

    if (methods.contains(QStringLiteral("GET"))) {
        methods.append(QStringLiteral("HEAD"));
    }

    methods.removeAll(QStringLiteral("not_implemented"));
    methods.sort();
    methods.removeDuplicates();

    return methods.join(QStringLiteral(", ")).toLatin1();
}
