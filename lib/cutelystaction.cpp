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

#include "cutelystaction.h"
#include "cutelystcontroller.h"
#include "cutelystcontext.h"

#include <QMetaClassInfo>
#include <QDebug>

CutelystAction::CutelystAction(const QMetaMethod &method, CutelystController *parent) :
    QObject(parent),
    m_method(method),
    m_controller(parent),
    m_numberOfArgs(0),
    m_numberOfCaptures(0)
{
    QString actionNamespace;
    // Parse the Method attributes declared with Q_CLASSINFO
    // They start with the method_name then
    // optionally followed by the number of arguments it takes
    // and finally the attribute name.
    QString regexString = QString("%1_(\\w+)").arg(QString(m_method.name()));
    QRegExp regex(regexString);
    for (int i = 0; i < parent->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = parent->metaObject()->classInfo(i);
        QString name = classInfo.name();
        if (name == QLatin1String("Namespace")) {
            actionNamespace = classInfo.value();
            continue;
        }

        if (regex.indexIn(name) != -1) {
            m_attributes.insertMulti(regex.cap(1), classInfo.value());
        }
    }

    if (!m_attributes.contains(QLatin1String("Args"))) {
        m_numberOfArgs = method.parameterCount();
        m_attributes.insertMulti(QLatin1String("Args"), QString::number(m_numberOfArgs));
    }

    if (!m_attributes.contains(QLatin1String("CaptureArgs"))) {
        m_numberOfCaptures = method.parameterCount();
        m_attributes.insertMulti(QLatin1String("CaptureArgs"), QString::number(m_numberOfCaptures));
    }

//    qDebug() << Q_FUNC_INFO << actionNamespace << m_attributes;
//    qDebug() << Q_FUNC_INFO << m_method.parameterTypes() << m_method.parameterNames();

////    if (m_attributes.contains(QLatin1String("Path"))) {
//////        if (m_attributes) {

//////        }
////    }
}

QMultiHash<QString, QString> CutelystAction::attributes() const
{
    return m_attributes;
}

QString CutelystAction::className() const
{
    return parent()->metaObject()->className();
}

CutelystController *CutelystAction::controller() const
{
    return m_controller;
}

bool CutelystAction::dispatch(CutelystContext *c)
{
    m_controller->setContext(c);

    QStringList args = c->args();
    // Fill the missing arguments
    for (int i = args.count(); i < 9; ++i) {
        args << QString();
    }

    return m_method.invoke(m_controller,
                           Q_ARG(QString, args.at(0)),
                           Q_ARG(QString, args.at(1)),
                           Q_ARG(QString, args.at(2)),
                           Q_ARG(QString, args.at(3)),
                           Q_ARG(QString, args.at(4)),
                           Q_ARG(QString, args.at(5)),
                           Q_ARG(QString, args.at(6)),
                           Q_ARG(QString, args.at(7)),
                           Q_ARG(QString, args.at(8)));
}

bool CutelystAction::match(CutelystContext *c) const
{
    if (m_attributes.contains(QLatin1String("Args")) &&
            m_attributes.value(QLatin1String("Args")).isEmpty()) {
        return true;
    }
    return m_numberOfArgs == 0 || m_numberOfArgs == c->args().size();
}

bool CutelystAction::matchCaptures(CutelystContext *c) const
{
    return m_numberOfCaptures == 0 || m_numberOfCaptures == c->args().size();
}

QString CutelystAction::name() const
{
    return m_method.name();
}

quint8 CutelystAction::numberOfArgs() const
{
    return m_numberOfArgs;
}

quint8 CutelystAction::numberOfCaptures() const
{
    return m_numberOfCaptures;
}
