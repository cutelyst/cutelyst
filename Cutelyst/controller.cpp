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

#include "controller.h"

#include "dispatcher.h"
#include "action.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QDebug>

using namespace Cutelyst;

Controller::Controller(QObject *parent) :
    QObject(parent)
{
    QByteArray controlerNS;
    for (int i = 0; i < metaObject()->classInfoCount(); ++i) {
        if (metaObject()->classInfo(i).name() == QLatin1String("Namespace")) {
            controlerNS = metaObject()->classInfo(i).value();
            break;
        }
    }

    if (controlerNS.isNull()) {
        QString className = metaObject()->className();
        bool lastWasUpper = true;

        for (int i = 0; i < className.length(); ++i) {
            if (className.at(i).toLower() == className.at(i)) {
                controlerNS.append(className.at(i));
                lastWasUpper = false;
            } else {
                if (lastWasUpper) {
                    controlerNS.append(className.at(i).toLower());
                } else {
                    controlerNS.append(QLatin1Char('/') % className.at(i).toLower());
                }
                lastWasUpper = true;
            }
        }
    }

    setProperty("ns", controlerNS);
}

Controller::~Controller()
{
}

QByteArray Controller::ns() const
{
    return property("ns").toByteArray();
}

Action *Controller::actionFor(Context *ctx, const QString &name)
{
    return ctx->dispatcher()->getAction(name, ns());
}

bool Controller::operator==(const char *className)
{
    return !qstrcmp(metaObject()->className(), className);
}

void Controller::Begin(Context *ctx)
{

}

bool Controller::Auto(Context *ctx)
{
    return true;
}

void Controller::End(Context *ctx)
{

}

void Controller::_DISPATCH(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    QStringList dispatchSteps;
    dispatchSteps << QLatin1String("_BEGIN");
    dispatchSteps << QLatin1String("_AUTO");
    dispatchSteps << QLatin1String("_ACTION");
    Q_FOREACH (const QString &disp, dispatchSteps) {
        if (!ctx->forward(disp)) {
            break;
        }
    }

    ctx->forward(QLatin1String("_END"));
}

bool Controller::_BEGIN(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    ActionList beginList;
    beginList = ctx->getActions(QLatin1String("Begin"), ctx->ns());
    if (!beginList.isEmpty()) {
        Action *begin = beginList.last();
        begin->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}

bool Controller::_AUTO(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    ActionList autoList = ctx->getActions(QLatin1String("Auto"), ctx->ns());
    Q_FOREACH (Action *autoAction, autoList) {
        if (!autoAction->dispatch(ctx)) {
            return false;
        }
    }
    return true;
}

bool Controller::_ACTION(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    if (ctx->action()) {
        return ctx->action()->dispatch(ctx);
    }
    return !ctx->error();
}

bool Controller::_END(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    ActionList endList;
    endList = ctx->getActions(QLatin1String("End"), ctx->ns());
    if (!endList.isEmpty()) {
        Action *end = endList.last();
        end->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}
