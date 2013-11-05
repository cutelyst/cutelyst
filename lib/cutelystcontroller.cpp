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

QString CutelystController::classNamespace() const
{
    QString ns;
    for (int i = 0; i < metaObject()->classInfoCount(); ++i) {
        if (metaObject()->classInfo(i).name() == QLatin1String("Namespace")) {
            ns = metaObject()->classInfo(i).value();
            break;
        }
    }

    QString className = metaObject()->className();
    if (ns.isNull()) {
        bool lastWasUpper = false;
        for (int i = 0; i < className.length(); ++i) {
            if (className.at(i).toLower() == className.at(i)) {
                ns.append(className.at(i));
                lastWasUpper = false;
            } else {
                if (lastWasUpper) {
                    ns.append(className.at(i).toLower());
                } else {
                    ns.append(QLatin1Char('/') % className.at(i).toLower());
                }
                lastWasUpper = true;
            }
        }
    }

    if (!ns.startsWith(QLatin1Char('/'))) {
        ns.prepend(QLatin1Char('/'));
    }

    return ns;
}

CutelystContext *CutelystController::c() const
{
    return m_c;
}

void CutelystController::setContext(CutelystContext *c)
{
    m_c = c;
}

#include "moc_cutelystcontroller.cpp"
