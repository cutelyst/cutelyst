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
    d_ptr(new ActionPrivate)
{
    Q_D(Action);

    // Setup the ns()
    parent->init();

    d->name = parent->ns() + '/' + method.name();
    d->ns =parent->ns();
    d->method = method;
    d->methodName = method.name();
    d->controller = parent;

    QString actionNamespace;
    // Parse the Method attributes declared with Q_CLASSINFO
    // They start with the method_name then
    // optionally followed by the number of arguments it takes
    // and finally the attribute name.
    QRegularExpression regex(d->methodName % QLatin1String("_(\\w+)"));
    for (int i = 0; i < parent->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = parent->metaObject()->classInfo(i);
        QString name = classInfo.name();
        if (name == QLatin1String("Namespace")) {
            actionNamespace = classInfo.value();
            continue;
        }

        QRegularExpressionMatch match = regex.match(name);
        if (match.hasMatch()) {
            QString type = match.captured(1);
            QString value = classInfo.value();
            if (type == QLatin1String("Path")) {
                if (value.isEmpty()) {
                    value = controller()->ns() % QLatin1String("/") % d->name;
                } else if (value.startsWith(QLatin1String("/"))) {
                    value = controller()->ns() % QLatin1String("/") % d->name % QLatin1String("/") % value;
                } else {
                    value = controller()->ns() % QLatin1String("/") % value;
                }
            }
            d->attributes.insertMulti(type.toLocal8Bit(),
                                      value.toLocal8Bit());
        }
    }

    if (method.access() == QMetaMethod::Private) {
        d->attributes.insertMulti("Private", QByteArray());
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
            QByteArray name = d->name;
            if (type == "Global") {
                if (!d->name.startsWith('/')) {
                    name.prepend('/');
                }
                d->attributes.insertMulti("Path", name);
            } else if (type == "Local") {
                d->attributes.insertMulti("Path", name);
            } else if (type == "Path") {
                d->attributes.insertMulti("Path", controller()->ns());
            } else if (type == "Args" && !d->attributes.contains("Args")) {
                d->numberOfArgs = parameterCount;
                d->attributes.insertMulti("Args", QByteArray::number(d->numberOfArgs));
            } else if (type == "CaptureArgs" && !d->attributes.contains("CaptureArgs")) {
                d->numberOfCaptures = parameterCount;
                d->attributes.insertMulti("CaptureArgs", QByteArray::number(d->numberOfCaptures));
            }
        }
    }
}

Action::~Action()
{
    delete d_ptr;
}

QMultiHash<QByteArray, QByteArray> Action::attributes() const
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

QByteArray Action::name() const
{
    Q_D(const Action);
    return d->methodName;
}

QByteArray Action::privateName() const
{
    Q_D(const Action);
    return d->name;
}

QByteArray Action::ns() const
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
