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
    m_controller(parent)
{
    QString actionNamespace;
    // Parse the Method attributes declared with Q_CLASSINFO
    // They start with the method_name then
    // optionally followed by the number of arguments it takes
    // and finally the attribute name.
    QString regexString = QString("%1(_\\d+)?_(\\w+)").arg(QString(m_method.name()));
    QRegExp regex(regexString);
    for (int i = 0; i < parent->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = parent->metaObject()->classInfo(i);
        QString name = classInfo.name();
        if (name == QLatin1String("Namespace")) {
            actionNamespace = classInfo.value();
            continue;
        }

        if (regex.indexIn(name) != -1) {
            if (!regex.cap(1).isEmpty() && m_method.parameterCount() != regex.cap(1).toInt()) {
                continue;
            }

            m_attributes[regex.cap(2)] = classInfo.value();
        }
    }
    qDebug() << Q_FUNC_INFO << actionNamespace << m_attributes;
    qDebug() << Q_FUNC_INFO << m_method.parameterTypes() << m_method.parameterNames();

    if (m_attributes.contains(QLatin1String("Path"))) {
//        if (m_attributes) {

//        }
    }
}

bool CutelystAction::isValid() const
{
    qDebug() << Q_FUNC_INFO << m_method.parameterTypes();
    return parent() &&
            m_method.parameterCount() &&
            m_method.parameterTypes().first() == "CutelystContext*";
}

QHash<QString, QString> CutelystAction::attributes() const
{

}

QString CutelystAction::className() const
{
    return parent()->objectName();
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

}

bool CutelystAction::matchCaptures(CutelystContext *c) const
{

}

QString CutelystAction::name() const
{

}

quint8 CutelystAction::numberOfArgs() const
{

}

quint8 CutelystAction::numberOfCaptures() const
{

}
