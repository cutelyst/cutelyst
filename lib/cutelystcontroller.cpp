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

#include "cutelystcontext.h"
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
        bool lastWasUpper = false;
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

    if (!ret.startsWith(QLatin1Char('/'))) {
        ret.prepend(QLatin1Char('/'));
    }

    return ret;
}

CutelystContext *CutelystController::c() const
{
    return m_c;
}

void CutelystController::setContext(CutelystContext *c)
{
    m_c = c;
}

void CutelystController::dispatchBegin()
{

}

void CutelystController::dispatchAuto()
{

}

void CutelystController::dispatchAction()
{

}

void CutelystController::dispatchEnd()
{

}

bool CutelystController::_DISPATCH()
{
    qDebug() << Q_FUNC_INFO;
    QStringList dispatchSteps;
    dispatchSteps << QLatin1String("_BEGIN");
    dispatchSteps << QLatin1String("_AUTO");
    dispatchSteps << QLatin1String("_ACTION");
    foreach (const QString &disp, dispatchSteps) {
        if (!m_c->forward(disp)) {
            break;
        }
    }

    m_c->forward(QLatin1String("_END"));
}

bool CutelystController::_BEGIN()
{
    qDebug() << Q_FUNC_INFO;
    CutelystAction *begin = m_c->getAction(QLatin1String("dispatchBegin"));
    if (begin) {
        begin->dispatch(m_c);
        return !m_c->error();
    }
    return true;
}

bool CutelystController::_AUTO()
{
    qDebug() << Q_FUNC_INFO;
    QList<CutelystAction*> autoList = m_c->getActions(QLatin1String("dispatchAuto"));
    foreach (CutelystAction *autoAction, autoList) {
        autoAction->dispatch(m_c);
        if (m_c->state()) {
            return false;
        }
    }
    return true;
}

bool CutelystController::_ACTION()
{
    qDebug() << Q_FUNC_INFO;
    if (m_c->action()) {
        return m_c->action()->dispatch(m_c);
    }
    return !m_c->error();
}

bool CutelystController::_END()
{
    qDebug() << Q_FUNC_INFO;
    CutelystAction *begin = m_c->getAction(QLatin1String("dispatchEnd"));
    if (begin) {
        begin->dispatch(m_c);
        return !m_c->error();
    }
    return true;
}

#include "moc_cutelystcontroller.cpp"
