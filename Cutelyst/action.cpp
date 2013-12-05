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

#include "action_p.h"
#include "controller.h"
#include "context.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QDebug>

using namespace Cutelyst;

Action::Action(const QMetaMethod &method, Controller *parent) :
    QObject(parent),
    d_ptr(new ActionPrivate(method, parent))
{
    Q_D(Action);

    QString actionNamespace;
    // Parse the Method attributes declared with Q_CLASSINFO
    // They start with the method_name then
    // optionally followed by the number of arguments it takes
    // and finally the attribute name.
    QRegularExpression regex(d->name % QLatin1String("_(\\w+)"));
    for (int i = 0; i < parent->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = parent->metaObject()->classInfo(i);
        QString name = classInfo.name();
        if (name == QLatin1String("Namespace")) {
            actionNamespace = classInfo.value();
            continue;
        }

        QRegularExpressionMatch match = regex.match(name);
        if (match.hasMatch()) {
            d->attributes.insertMulti(match.captured(1), classInfo.value());
        }
    }

    if (method.access() == QMetaMethod::Private) {
        d->attributes.insertMulti(QLatin1String("Private"), QString());
    }

    // if the method has the CaptureArgs as an argument
    // set it on the attributes
    int parameterCount = 0;
    bool ignoreParameters = false;
    QList<QByteArray> parameterTypes = method.parameterTypes();
    for (int i = 0; i < parameterTypes.size(); ++i) {
        const QByteArray &type = parameterTypes.at(i);

        if (i == 0) {
            if (!type.endsWith("Context*")) {
                d->valid = false;
                return;
            }
        } else if (type == "QString" && !ignoreParameters) {
            ++parameterCount;
        } else {
            // Make sure the user defines specia
            // parameters types AFTER the ones to be captured
            ignoreParameters = true;
            if (type == "Global") {
                if (d->name.startsWith(QLatin1Char('/'))) {
                    d->attributes.insertMulti(QLatin1String("Path"), d->name);
                } else {
                    d->attributes.insertMulti(QLatin1String("Path"), QLatin1Char('/') % d->name);
                }
            } else if (type == "Local") {
                d->attributes.insertMulti(QLatin1String("Path"), d->name);
            } else if (type == "Path") {
                d->attributes.insertMulti(QLatin1String("Path"), controller()->ns());
            } else if (type == "Args" && !d->attributes.contains(QLatin1String("Args"))) {
                d->numberOfArgs = parameterCount;
                d->attributes.insertMulti(QLatin1String("Args"), QString::number(d->numberOfArgs));
            } else if (type == "CaptureArgs" && !d->attributes.contains(QLatin1String("CaptureArgs"))) {
                d->numberOfCaptures = parameterCount;
                d->attributes.insertMulti(QLatin1String("Args"), QString::number(d->numberOfCaptures));
            }
        }
    }
}

Action::~Action()
{
    delete d_ptr;
}

QMultiHash<QString, QString> Action::attributes() const
{
    Q_D(const Action);
    return d->attributes;
}

QString Action::className() const
{
    return parent()->metaObject()->className();
}

Controller *Action::controller() const
{
    Q_D(const Action);
    return d->controller;
}

bool Action::dispatch(Context *ctx)
{
    Q_D(Action);

    if (ctx->detached()) {
        return false;
    }

    QStringList args = ctx->args();
    // Fill the missing arguments
    for (int i = args.count(); i < 8; ++i) {
        args << QString();
    }

    if (d->method.returnType() == QMetaType::Bool) {
        bool methodRet;
        bool ret;
        ret = d->method.invoke(d->controller,
                               Q_RETURN_ARG(bool, methodRet),
                               Q_ARG(Context*, ctx),
                               Q_ARG(QString, args.at(0)),
                               Q_ARG(QString, args.at(1)),
                               Q_ARG(QString, args.at(2)),
                               Q_ARG(QString, args.at(3)),
                               Q_ARG(QString, args.at(4)),
                               Q_ARG(QString, args.at(5)),
                               Q_ARG(QString, args.at(6)),
                               Q_ARG(QString, args.at(7)));

        if (ret) {
            ctx->setState(methodRet);
            return methodRet;
        }

        // TODO when the method failed to be called it probably means
        // we should detach, make sure this would be enough
        ctx->detach();
        ctx->setState(false);

        return false;
    } else {
        bool ret = d->method.invoke(d->controller,
                                    Q_ARG(Context*, ctx),
                                    Q_ARG(QString, args.at(0)),
                                    Q_ARG(QString, args.at(1)),
                                    Q_ARG(QString, args.at(2)),
                                    Q_ARG(QString, args.at(3)),
                                    Q_ARG(QString, args.at(4)),
                                    Q_ARG(QString, args.at(5)),
                                    Q_ARG(QString, args.at(6)),
                                    Q_ARG(QString, args.at(7)));
        ctx->setState(ret);
        return ret;
    }
}

bool Action::match(Context *ctx) const
{
    Q_D(const Action);
    if (d->attributes.contains(QLatin1String("Args")) &&
            d->attributes.value(QLatin1String("Args")).isEmpty()) {
        return true;
    }
    return d->numberOfArgs == 0 || d->numberOfArgs == ctx->args().size();
}

bool Action::matchCaptures(Context *ctx) const
{
    Q_D(const Action);
    return d->numberOfCaptures == 0 || d->numberOfCaptures == ctx->args().size();
}

QString Action::name() const
{
    Q_D(const Action);
    return d->method.name();
}

QString Action::privateName() const
{
    Q_D(const Action);
    return d->name;
}

QString Action::ns() const
{
    Q_D(const Action);
    return d->ns;
}

quint8 Action::numberOfArgs() const
{
    Q_D(const Action);
    return d->numberOfArgs;
}

quint8 Action::numberOfCaptures() const
{
    Q_D(const Action);
    return d->numberOfCaptures;
}

bool Action::isValid() const
{
    Q_D(const Action);
    return d->valid;
}


ActionPrivate::ActionPrivate(const QMetaMethod &method, Controller *parent) :
    valid(true),
    name(parent->ns() % QLatin1Char('/') % method.name()),
    ns(parent->ns()),
    method(method),
    controller(parent),
    numberOfArgs(0),
    numberOfCaptures(0)
{

}
