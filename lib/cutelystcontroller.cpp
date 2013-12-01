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

#include "cutelystcontroller.h"

#include "cutelystdispatcher.h"
#include "cutelystaction.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QDebug>

CutelystController::CutelystController(QObject *parent) :
    QObject(parent)
{
}

CutelystController::~CutelystController()
{
}

QString CutelystController::ns() const
{
    QString ret;
    for (int i = 0; i < metaObject()->classInfoCount(); ++i) {
        if (metaObject()->classInfo(i).name() == QLatin1String("Namespace")) {
            ret = metaObject()->classInfo(i).value();
            break;
        }
    }

    QString className = metaObject()->className();
    if (ret.isNull()) {
        bool lastWasUpper = true;
        for (int i = 0; i < className.length(); ++i) {
            if (className.at(i).toLower() == className.at(i)) {
                ret.append(className.at(i));
                lastWasUpper = false;
            } else {
                if (lastWasUpper) {
                    ret.append(className.at(i).toLower());
                } else {
                    ret.append(QLatin1Char('/') % className.at(i).toLower());
                }
                lastWasUpper = true;
            }
        }
    }

    return ret;
}

CutelystAction *CutelystController::actionFor(Cutelyst *c, const QString &name)
{
    return c->dispatcher()->getAction(name, ns());
}

bool CutelystController::operator==(const char *className)
{
    return !qstrcmp(metaObject()->className(), className);
}

void CutelystController::Begin(Cutelyst *c)
{

}

bool CutelystController::Auto(Cutelyst *c)
{
    return true;
}

void CutelystController::End(Cutelyst *c)
{

}

void CutelystController::Default(Cutelyst *c)
{

}

void CutelystController::Index(Cutelyst *c)
{

}

void CutelystController::_DISPATCH(Cutelyst *c)
{
//    qDebug() << Q_FUNC_INFO;
    QStringList dispatchSteps;
    dispatchSteps << QLatin1String("_BEGIN");
    dispatchSteps << QLatin1String("_AUTO");
    dispatchSteps << QLatin1String("_ACTION");
    foreach (const QString &disp, dispatchSteps) {
        if (!c->forward(disp)) {
            break;
        }
    }

    c->forward(QLatin1String("_END"));
}

bool CutelystController::_BEGIN(Cutelyst *c)
{
//    qDebug() << Q_FUNC_INFO;
    QList<CutelystAction*> beginList;
    beginList = c->getActions(QLatin1String("Begin"), c->ns());
    if (!beginList.isEmpty()) {
        CutelystAction *begin = beginList.last();
        begin->dispatch(c);
        return !c->error();
    }
    return true;
}

bool CutelystController::_AUTO(Cutelyst *c)
{
//    qDebug() << Q_FUNC_INFO;
    QList<CutelystAction*> autoList = c->getActions(QLatin1String("Auto"), c->ns());
    foreach (CutelystAction *autoAction, autoList) {
        if (!autoAction->dispatch(c)) {
            return false;
        }
    }
    return true;
}

bool CutelystController::_ACTION(Cutelyst *c)
{
//    qDebug() << Q_FUNC_INFO;
    if (c->action()) {
        return c->action()->dispatch(c);
    }
    return !c->error();
}

bool CutelystController::_END(Cutelyst *c)
{
//    qDebug() << Q_FUNC_INFO;
    QList<CutelystAction*> endList;
    endList = c->getActions(QLatin1String("End"), c->ns());
    if (!endList.isEmpty()) {
        CutelystAction *end = endList.last();
        end->dispatch(c);
        return !c->error();
    }
    return true;
}

#include "moc_cutelystcontroller.cpp"
