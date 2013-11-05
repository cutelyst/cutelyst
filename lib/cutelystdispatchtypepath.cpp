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

#include "cutelystdispatchtypepath.h"

#include "cutelystaction.h"

#include <QDebug>

CutelystDispatchTypePath::CutelystDispatchTypePath(QObject *parent) :
    CutelystDispatchType(parent)
{
}

void CutelystDispatchTypePath::list() const
{

}

bool CutelystDispatchTypePath::match(const QUrl &path) const
{

}

bool CutelystDispatchTypePath::registerAction(CutelystAction *action)
{
    QMultiHash<QString, QString> attributes = action->attributes();
    QMultiHash<QString, QString>::iterator i = attributes.find(QLatin1String("Path"));
    while (i != attributes.end() && i.key() == QLatin1String("Path")) {
        qDebug() << Q_FUNC_INFO << i.value();
        ++i;
    }
}
