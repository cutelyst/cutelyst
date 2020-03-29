/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "cutelystcutelee.h"

#include "urifor.h"
#include "csrf.h"

CutelystCutelee::CutelystCutelee(QObject *parent) : QObject(parent)
{
}

QHash<QString, Cutelee::AbstractNodeFactory *> CutelystCutelee::nodeFactories(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Cutelee::AbstractNodeFactory *> ret {
        {QStringLiteral("c_uri_for"), new UriForTag()},
        {QStringLiteral("c_csrf_token"), new CSRFTag()},
    };

    return ret;
}

QHash<QString, Cutelee::Filter *> CutelystCutelee::filters(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Cutelee::Filter *> ret;

    return ret;
}

#include "moc_cutelystcutelee.cpp"
