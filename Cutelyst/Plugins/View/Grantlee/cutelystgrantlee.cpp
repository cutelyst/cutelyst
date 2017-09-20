/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "cutelystgrantlee.h"

#include "urifor.h"

CutelystGrantlee::CutelystGrantlee(QObject *parent) : QObject(parent)
{
}

QHash<QString, Grantlee::AbstractNodeFactory *> CutelystGrantlee::nodeFactories(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::AbstractNodeFactory *> ret;

    ret.insert(QStringLiteral("c_uri_for"), new UriForTag());

    return ret;
}

QHash<QString, Grantlee::Filter *> CutelystGrantlee::filters(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::Filter *> ret;

    return ret;
}

#include "moc_cutelystgrantlee.cpp"
