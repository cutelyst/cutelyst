/*
 * Copyright (C) 2013-2018 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "actionrest_p.h"
#include "context.h"
#include "controller.h"
#include "dispatcher.h"

#include <QUrl>
#include <QDebug>

using namespace Cutelyst;

/*!
 * \class Cutelyst::ActionREST actionrest.h Cutelyst/Actions/REST/ActionREST
 * \brief Automated REST Method Dispatching
 *
 * \code{.h}
 * C_ATTR(foo, :Local :ActionClass(REST))
 * void foo(Context *c); // do setup for HTTP method specific handlers
 *
 * C_ATTR(foo_GET, :Private)
 * void foo_GET(Context *c); // do something for GET requests
 *
 * C_ATTR(foo_PUT, :Private)
 * void foo_PUT(Context *c); // do something for PUT requests
 * \endcode
 *
 * This Action handles doing automatic method dispatching for REST requests. It takes a normal Cutelyst action, and changes
 * the dispatch to append an underscore and method name. First it will try dispatching to an action with the generated name,
 * and failing that it will try to dispatch to a regular method.
 *
 * For example, in the synopsis above, calling GET on "/foo" would result in the foo_GET method being dispatched.
 *
 * If a method is requested that is not implemented, this action will return a status 405 (Method Not Found). It will populate
 * the "Allow" header with the list of implemented request methods. You can override this behavior by implementing a custom
 * 405 handler like so:
 *
 * \code{.h}
 * C_ATTR(foo_not_implemented, :Private)
 * void foo_not_implemented(Context *c); // handle not implemented methods
 * \endcode
 *
 * If you do not provide an _OPTIONS method, we will automatically respond with a 200 OK. The "Allow" header will be populated
 * with the list of implemented request methods. If you do not provide an _HEAD either, we will auto dispatch to the _GET one
 * in case it exists.
 */
ActionREST::ActionREST(QObject *parent) : Action(new ActionRESTPrivate(this), parent)
{
}

bool ActionREST::doExecute(Context *c)
{
    Q_D(const ActionREST);

    if (!Action::doExecute(c)) {
        return false;
    }

    return d->dispatchRestMethod(c, c->request()->method());
}

ActionRESTPrivate::ActionRESTPrivate(ActionREST* q) : q_ptr(q)
{
}

bool ActionRESTPrivate::dispatchRestMethod(Context *c, const QString &httpMethod) const
{
    Q_Q(const ActionREST);
    const QString restMethod = q->name() + QLatin1Char('_') + httpMethod;

    Controller *controller = q->controller();
    Action *action = controller->actionFor(restMethod);
    if (!action) {
        // Look for non registered actions in this controller
        const ActionList actions = controller->actions();
        for (Action *controllerAction : actions) {
            if (controllerAction->name() == restMethod) {
                action = controllerAction;
                break;
            }
        }
    }

    if (action) {
        return c->execute(action);
    }

    bool ret = false;
    if (httpMethod == QLatin1String("OPTIONS")) {
        ret = returnOptions(c, q->name());
    } else if (httpMethod == QLatin1String("HEAD")) {
        // redispatch to GET
        ret = dispatchRestMethod(c, QStringLiteral("GET"));
    } else if (httpMethod != QLatin1String("not_implemented")) {
        // try dispatching to foo_not_implemented
        ret = dispatchRestMethod(c, QStringLiteral("not_implemented"));
    } else {
        // not_implemented
        ret = returnNotImplemented(c, q->name());
    }

    return ret;
}

bool ActionRESTPrivate::returnOptions(Context *c, const QString &methodName) const
{
    Response *response = c->response();
    response->setContentType(QStringLiteral("text/plain"));
    response->setStatus(Response::OK); // 200
    response->setHeader(QStringLiteral("ALLOW"),
                        getAllowedMethods(c->controller(), methodName));
    response->body().clear();
    return true;
}

bool ActionRESTPrivate::returnNotImplemented(Context *c, const QString &methodName) const
{
    Response *response = c->response();
    response->setStatus(Response::MethodNotAllowed); // 405
    response->setHeader(QStringLiteral("ALLOW"),
                        getAllowedMethods(c->controller(), methodName));
    const QString body = QLatin1String("Method ") + c->req()->method()
            + QLatin1String(" not implemented for ") + c->uriFor(methodName).toString();
    response->setBody(body);
    return true;
}

QString Cutelyst::ActionRESTPrivate::getAllowedMethods(Controller *controller, const QString &methodName) const
{
    QStringList methods;
    const QString name = methodName + QLatin1Char('_');
    const ActionList actions = controller->actions();
    for (Action *action : actions) {
        const QString method = action->name();
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

    return methods.join(QStringLiteral(", "));
}

#include "moc_actionrest.cpp"
