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
#include "cutelyst.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QDebug>

CutelystAction::CutelystAction(const QMetaMethod &method, CutelystController *parent) :
    QObject(parent),
    m_valid(true),
    m_name(parent->ns() % QLatin1Char('/') % method.name()),
    m_ns(parent->ns()),
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
    QRegularExpression regex(m_name % QLatin1String("_(\\w+)"));
    for (int i = 0; i < parent->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = parent->metaObject()->classInfo(i);
        QString name = classInfo.name();
        if (name == QLatin1String("Namespace")) {
            actionNamespace = classInfo.value();
            continue;
        }

        QRegularExpressionMatch match = regex.match(name);
        if (match.hasMatch()) {
            m_attributes.insertMulti(match.captured(1), classInfo.value());
        }
    }

    if (m_method.access() == QMetaMethod::Private) {
        m_attributes.insertMulti(QLatin1String("Private"), QString());
    }

    // if the method has the CaptureArgs as an argument
    // set it on the attributes
    int parameterCount = 0;
    bool ignoreParameters = false;
    QList<QByteArray> parameterTypes = method.parameterTypes();
    for (int i = 0; i < parameterTypes.size(); ++i) {
        const QByteArray &type = parameterTypes.at(i);

        if (i == 0) {
            if (type != "Cutelyst*") {
                m_valid = false;
                return;
            }
        } else if (type == "QString" && !ignoreParameters) {
            ++parameterCount;
        } else {
            // Make sure the user defines specia
            // parameters types AFTER the ones to be captured
            ignoreParameters = true;
            if (type == "Global") {
                if (m_name.startsWith(QLatin1Char('/'))) {
                    m_attributes.insertMulti(QLatin1String("Path"), m_name);
                } else {
                    m_attributes.insertMulti(QLatin1String("Path"), QLatin1Char('/') % m_name);
                }
            } else if (type == "Local") {
                m_attributes.insertMulti(QLatin1String("Path"), m_name);
            } else if (type == "Path") {
                m_attributes.insertMulti(QLatin1String("Path"), controller()->ns());
            } else if (type == "Args" && !m_attributes.contains(QLatin1String("Args"))) {
                m_numberOfArgs = parameterCount;
                m_attributes.insertMulti(QLatin1String("Args"), QString::number(m_numberOfArgs));
            } else if (type == "CaptureArgs" && !m_attributes.contains(QLatin1String("CaptureArgs"))) {
                m_numberOfCaptures = parameterCount;
                m_attributes.insertMulti(QLatin1String("Args"), QString::number(m_numberOfCaptures));
            }
        }
    }
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

bool CutelystAction::dispatch(Cutelyst *c)
{
    if (c->detached()) {
        return false;
    }

    QStringList args = c->args();
    // Fill the missing arguments
    for (int i = args.count(); i < 8; ++i) {
        args << QString();
    }

    if (m_method.returnType() == QMetaType::Bool) {
        bool methodRet;
        bool ret;
        ret = m_method.invoke(m_controller,
                              Q_RETURN_ARG(bool, methodRet),
                              Q_ARG(Cutelyst*, c),
                              Q_ARG(QString, args.at(0)),
                              Q_ARG(QString, args.at(1)),
                              Q_ARG(QString, args.at(2)),
                              Q_ARG(QString, args.at(3)),
                              Q_ARG(QString, args.at(4)),
                              Q_ARG(QString, args.at(5)),
                              Q_ARG(QString, args.at(6)),
                              Q_ARG(QString, args.at(7)));

        c->setState(ret);
        if (ret) {
            return methodRet;
        }

        // TODO when the method returns false it probably means
        // we should detach, make sure this would be enough
        c->detach();

        return false;
    } else {
        bool ret = m_method.invoke(m_controller,
                                   Q_ARG(Cutelyst*, c),
                                   Q_ARG(QString, args.at(0)),
                                   Q_ARG(QString, args.at(1)),
                                   Q_ARG(QString, args.at(2)),
                                   Q_ARG(QString, args.at(3)),
                                   Q_ARG(QString, args.at(4)),
                                   Q_ARG(QString, args.at(5)),
                                   Q_ARG(QString, args.at(6)),
                                   Q_ARG(QString, args.at(7)));
        c->setState(ret);
        return ret;
    }
}

bool CutelystAction::match(Cutelyst *c) const
{
    if (m_attributes.contains(QLatin1String("Args")) &&
            m_attributes.value(QLatin1String("Args")).isEmpty()) {
        return true;
    }
    return m_numberOfArgs == 0 || m_numberOfArgs == c->args().size();
}

bool CutelystAction::matchCaptures(Cutelyst *c) const
{
    return m_numberOfCaptures == 0 || m_numberOfCaptures == c->args().size();
}

QString CutelystAction::name() const
{
    return m_method.name();
}

QString CutelystAction::privateName() const
{
    return m_name;
}

QString CutelystAction::ns() const
{
    return m_ns;
}

quint8 CutelystAction::numberOfArgs() const
{
    return m_numberOfArgs;
}

quint8 CutelystAction::numberOfCaptures() const
{
    return m_numberOfCaptures;
}

bool CutelystAction::isValid() const
{
    return m_valid;
}
