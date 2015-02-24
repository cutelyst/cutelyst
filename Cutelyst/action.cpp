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

#include "action_p.h"
#include "controller.h"
#include "context.h"
#include "common.h"

using namespace Cutelyst;

Action::Action(QObject *parent) : Code(parent)
    , d_ptr(new ActionPrivate)
{
}

Action::~Action()
{
    delete d_ptr;
}

Code::Modifiers Action::modifiers() const
{
    return Code::OnlyExecute;
}

void Action::setMethod(const QMetaMethod &method)
{
    Q_D(Action);
    d->method = method;
}

void Action::setController(Controller *controller)
{
    Q_D(Action);
    d->controller = controller;
}

void Action::setupAction(const QVariantHash &args, Application *app)
{
    Q_D(Action);

    init(app, args);

    d->name = args.value("name").toString();
    d->ns = args.value("namespace").toString();
    d->reverse = args.value("reverse").toString();

    QMap<QString, QString> attributes = args.value("attributes").value<QMap<QString, QString> >();
    d->attributes = attributes;

    if (attributes.contains("Args")) {
        d->numberOfArgs = attributes.value("Args").toInt();
    }

    if (attributes.contains("CaptureArgs")) {
        d->numberOfCaptures = attributes.value("CaptureArgs").toInt();
    }
}

QMap<QString, QString> Action::attributes() const
{
    Q_D(const Action);
    return d->attributes;
}

QString Action::className() const
{
    Q_D(const Action);
    return d->controller->metaObject()->className();
}

Controller *Action::controller() const
{
    Q_D(const Action);
    return d->controller;
}

bool Action::dispatch(Context *ctx)
{
    return ctx->execute(this);
}

bool Action::match(int numberOfArgs) const
{
    Q_D(const Action);
    // If the number of args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfArgs == -1 || d->numberOfArgs == numberOfArgs;
}

bool Action::matchCaptures(int numberOfCaptures) const
{
    Q_D(const Action);
    // If the number of capture args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfCaptures == -1 || d->numberOfCaptures == numberOfCaptures;
}

QString Action::name() const
{
    Q_D(const Action);
    return d->name;
}

QString Action::reverse() const
{
    Q_D(const Action);
    return d->reverse;
}

QString Action::ns() const
{
    Q_D(const Action);
    return d->ns;
}

qint8 Action::numberOfArgs() const
{
    Q_D(const Action);
    return d->numberOfArgs;
}

qint8 Action::numberOfCaptures() const
{
    Q_D(const Action);
    return d->numberOfCaptures;
}

bool Action::doExecute(Context *ctx)
{
    Q_D(const Action);
    if (ctx->detached()) {
        return false;
    }

    QStringList args = ctx->request()->args();
    // Fill the missing arguments
    args += d->emptyArgs;

    if (d->method.returnType() == QMetaType::Bool) {
        bool methodRet;
        bool ret;
        ret = d->method.invoke(d->controller,
                               Qt::DirectConnection,
                               Q_RETURN_ARG(bool, methodRet),
                               Q_ARG(Cutelyst::Context*, ctx),
                               Q_ARG(QString, args.at(0)),
                               Q_ARG(QString, args.at(1)),
                               Q_ARG(QString, args.at(2)),
                               Q_ARG(QString, args.at(3)),
                               Q_ARG(QString, args.at(4)),
                               Q_ARG(QString, args.at(5)),
                               Q_ARG(QString, args.at(6)),
                               Q_ARG(QString, args.at(7)),
                               Q_ARG(QString, args.at(8)));

        if (ret) {
            ctx->setState(methodRet);
            return methodRet;
        }

        // The method failed to be called which means we should detach
        ctx->detach();
        ctx->setState(false);

        return false;
    } else {
        bool ret = d->method.invoke(d->controller,
                                    Qt::DirectConnection,
                                    Q_ARG(Cutelyst::Context*, ctx),
                                    Q_ARG(QString, args.at(0)),
                                    Q_ARG(QString, args.at(1)),
                                    Q_ARG(QString, args.at(2)),
                                    Q_ARG(QString, args.at(3)),
                                    Q_ARG(QString, args.at(4)),
                                    Q_ARG(QString, args.at(5)),
                                    Q_ARG(QString, args.at(6)),
                                    Q_ARG(QString, args.at(7)),
                                    Q_ARG(QString, args.at(8)));
        ctx->setState(ret);
        return ret;
    }
}
